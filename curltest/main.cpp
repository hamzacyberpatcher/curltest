#include <iostream>
#include <string>
#include <string.h>
#include <curl/curl.h>
#include <fstream>
#include "nlohmann/json.hpp"

// A callback function to store the HTTP response data in a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

int main() {
    // Initialize CURL
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();

    std::string url = "https://pokeapi.co/api/v2/pokemon/";

    std::string pokemon;

    std::cout << "Enter the pokemon name or pokdex number: ";
    std::cin >> pokemon;

    url = url + pokemon;


    if (curl) {
        std::string readBuffer;

        // Set the URL for the API
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        // Pass the response string to the callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            // Parse the JSON response
            try {
                nlohmann::json jsonData = nlohmann::json::parse(readBuffer);

                // Access data from the JSON object
                std::cout << "Name: " << jsonData["name"] << std::endl;
                std::cout << "Base Experience: " << jsonData["base_experience"] << std::endl;

                // Access abilities (array in JSON)
                std::cout << "Abilities:" << std::endl;
                for (const auto& ability : jsonData["abilities"]) {
                    std::cout << "- " << ability["ability"]["name"] << std::endl;
                }
                std::string sprite_url = jsonData["sprites"]["other"]["official-artwork"]["front_default"].get<std::string>();
                
                // Download the sprite
                curl_easy_setopt(curl, CURLOPT_URL, sprite_url.c_str());
                readBuffer.clear();
                res = curl_easy_perform(curl);
                
                if (res != CURLE_OK) {
                    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                }
                else {
                    // Write the sprite data to a file
                    std::ofstream outfile("pokemon_sprite.png", std::ios::binary);
                    outfile.write(readBuffer.data(), readBuffer.size());
                    outfile.close();
                
                    std::cout << "Sprite downloaded successfully!" << std::endl;
                }

                std::string cry_url = jsonData["cries"]["latest"].get<std::string>();
                
                // Download the cry using the same curl handle
                curl_easy_setopt(curl, CURLOPT_URL, cry_url.c_str());
                readBuffer.clear();
                res = curl_easy_perform(curl);
                
                if (res != CURLE_OK) {
                    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                }
                else {
                    // Save the cry to a file
                    std::ofstream outfile("pokemon_cry.ogg", std::ios::binary);
                    outfile.write(readBuffer.data(), readBuffer.size());
                    outfile.close();
                
                    std::cout << "Cry downloaded successfully!" << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            }
        }

        // Cleanup
        curl_easy_cleanup(curl);
    }

    system("pause");
    return 0;
}