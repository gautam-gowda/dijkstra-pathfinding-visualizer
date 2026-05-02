#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <ctime>
using namespace std;

// Graph
map<string, vector<pair<string,int>>> graph;
map<string, sf::Vector2f> pos;
map<string,string> parent;
map<string,int> distFinal;

string source="", dest="";
vector<string> path;
int step = 0;
bool validPath = false;

sf::Clock animClock;

// ---------------- RESET ----------------
void resetSelection() {
    source = "";
    dest = "";
    path.clear();
    step = 0;
    validPath = false;
}

// ---------------- DIJKSTRA ----------------
void dijkstra(string src) {
    map<string,int> dist;

    for(auto &p : graph)
        dist[p.first] = INT_MAX;

    priority_queue<pair<int,string>, vector<pair<int,string>>, greater<>> pq;

    dist[src] = 0;
    pq.push({0, src});
    parent.clear();

    while(!pq.empty()) {
        auto top = pq.top(); pq.pop();
        int d = top.first;
        string u = top.second;

        for(auto &edge : graph[u]) {
            string v = edge.first;
            int w = edge.second;

            if(dist[v] > d + w) {
                dist[v] = d + w;
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    distFinal = dist;
}

// ---------------- PATH ----------------
vector<string> getPath(string d) {
    vector<string> p;

    if(distFinal[d] == INT_MAX) return p;

    while(d != "") {
        p.push_back(d);
        d = parent[d];
    }

    reverse(p.begin(), p.end());
    return p;
}

// ---------------- MAIN ----------------
int main() {

    srand(time(0));

    int n, m;

    cout << "Enter number of cities: ";
    cin >> n;

    vector<string> cities(n);

    cout << "Enter city names:\n";
    for(int i=0; i<n; i++) {
        cin >> cities[i];

        // RANDOM SCATTER
        bool placed = false;

        while(!placed) {
            float x = 100 + rand() % 800;
            float y = 100 + rand() % 400;

            placed = true;

            for(auto &p : pos) {
                float dx = x - p.second.x;
                float dy = y - p.second.y;

                if(sqrt(dx*dx + dy*dy) < 120) {
                    placed = false;
                    break;
                }
            }

            if(placed)
                pos[cities[i]] = {x, y};
        }
    }

    cout << "Enter number of roads: ";
    cin >> m;

    cout << "Enter roads (city1 city2 distance in km):\n";
    for(int i=0; i<m; i++) {
        string a, b;
        int d;
        cin >> a >> b >> d;

        graph[a].push_back({b, d});
        graph[b].push_back({a, d});
    }

    cout << "\n✔ Click cities in window (Press R to reset)\n";

    sf::RenderWindow window(sf::VideoMode(1000, 600), "Navigation System");
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontLoaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    while(window.isOpen()) {

        sf::Event event;
        while(window.pollEvent(event)) {

            if(event.type == sf::Event::Closed)
                window.close();

            // RESET KEY
            if(event.type == sf::Event::KeyPressed) {
                if(event.key.code == sf::Keyboard::R) {
                    resetSelection();
                }
            }

            // MOUSE CLICK
            if(event.type == sf::Event::MouseButtonPressed) {

                sf::Vector2f mouse = window.mapPixelToCoords(
                    sf::Mouse::getPosition(window)
                );

                for(auto &p : pos) {

                    float dx = mouse.x - p.second.x;
                    float dy = mouse.y - p.second.y;

                    if(sqrt(dx*dx + dy*dy) < 15) {

                        // If both selected → reset automatically
                        if(source != "" && dest != "") {
                            resetSelection();
                        }

                        if(source == "") {
                            source = p.first;
                        }
                        else if(dest == "" && p.first != source) {

                            dest = p.first;

                            path.clear();
                            step = 0;
                            validPath = false;

                            dijkstra(source);
                            path = getPath(dest);

                            if(distFinal[dest] != INT_MAX)
                                validPath = true;
                        }
                    }
                }
            }
        }

        // ===== UI =====
        window.clear(sf::Color(18,18,24));

        // GRID
        for(int i=0;i<1000;i+=50){
            sf::Vertex v[] = {
                sf::Vertex(sf::Vector2f(i,0), sf::Color(40,40,50)),
                sf::Vertex(sf::Vector2f(i,600), sf::Color(40,40,50))
            };
            window.draw(v,2,sf::Lines);
        }

        // ROADS
        for(auto &u : graph) {
            for(auto &v : u.second) {
                sf::Vertex line[] = {
                    sf::Vertex(pos[u.first], sf::Color(90,90,110)),
                    sf::Vertex(pos[v.first], sf::Color(90,90,110))
                };
                window.draw(line, 3, sf::Lines);
            }
        }

        // ANIMATION
        if(validPath && path.size() >= 2 &&
           animClock.getElapsedTime().asSeconds() > 0.4 &&
           step < (int)path.size()-1) {

            step++;
            animClock.restart();
        }

        if(validPath && path.size() >= 2) {
            for(int i=0;i<step && i < (int)path.size()-1;i++) {

                sf::Vertex line[] = {
                    sf::Vertex(pos[path[i]], sf::Color(255,90,90)),
                    sf::Vertex(pos[path[i+1]], sf::Color(255,90,90))
                };

                window.draw(line, 5, sf::Lines);
            }
        }

        // CITIES
        for(auto &p : pos) {

            sf::CircleShape glow(22);
            glow.setOrigin(22,22);
            glow.setPosition(p.second);
            glow.setFillColor(sf::Color(0,255,150,40));
            window.draw(glow);

            sf::CircleShape circle(14);
            circle.setOrigin(14,14);
            circle.setPosition(p.second);

            if(p.first == source)
                circle.setFillColor(sf::Color(0,120,255));
            else if(p.first == dest)
                circle.setFillColor(sf::Color(255,200,0));
            else
                circle.setFillColor(sf::Color(0,255,150));

            window.draw(circle);

            if(fontLoaded) {
                sf::Text text(p.first, font, 15);
                text.setPosition(p.second.x + 18, p.second.y - 10);
                window.draw(text);
            }
        }

        // PANEL
        sf::RectangleShape panel(sf::Vector2f(1000,50));
        panel.setPosition(0,550);
        panel.setFillColor(sf::Color(35,35,45));
        window.draw(panel);

        if(fontLoaded) {
            sf::Text info("Click cities | Press R to reset", font, 16);
            info.setPosition(20,565);
            window.draw(info);
        }

        // RESULT
        if(source != "" && dest != "" && fontLoaded) {

            sf::RectangleShape box(sf::Vector2f(350,40));
            box.setPosition(630,555);
            box.setFillColor(sf::Color(50,50,60));
            window.draw(box);

            sf::Text t;

            if(validPath) {
                t.setString("Distance: " + to_string(distFinal[dest]) + " km");
                t.setFillColor(sf::Color::Cyan);
            } else {
                t.setString("Invalid Route");
                t.setFillColor(sf::Color::Red);
            }

            t.setFont(font);
            t.setCharacterSize(18);
            t.setPosition(640,562);

            window.draw(t);
        }

        window.display();
    }

    return 0;
}