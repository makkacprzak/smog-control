#include "translate.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>
#include <unordered_set>

using json = nlohmann::json;
using namespace std;

Translate::Translate(const std::string& phrase, const std::string& lang){
    if(lang != "pl"){
        if(ifstream fileIn("dictionary.json"); !fileIn.is_open()){
            throw runtime_error("Brak pliku słownika");
        }else{
            string myWord;
            json dictionary;
            fileIn >> dictionary;
            if(dictionary.contains(phrase) && dictionary[phrase].contains(lang)){
                translated_ = dictionary[phrase][lang].get<string>();
            }else{
                translated_ = phrase;
            }
        }
    }else{
        translated_ = phrase;
    }
}

Translate::operator string() const{
    return translated_;
}

vector<string> Translate::getLangs(){
    unordered_set<string> langSet;
    if(ifstream fileIn("dictionary.json"); !fileIn.is_open()){
        throw runtime_error("Brak pliku słownika");
    }else{
        json dictionary;
        fileIn >> dictionary;
        for (const auto& [key, value] : dictionary.items()) {
            if (value.is_object()) {
                for (const auto& [lang, translation] : value.items()) {
                    langSet.insert(lang);
                }
            }
        }
    }
    langSet.insert("pl");
    return vector<string>(langSet.begin(), langSet.end());
}
