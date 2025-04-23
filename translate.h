#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <string>
#include <vector>
/**
 * @brief Class for handling localisation of the app
 * @details A permanent object of this class never gets created. It is used only to translate a given word to a given language
 * using a pre-made dictionary. It is made in a way that allows adding future languages.
 */
class Translate
{
private:
    ///@brief Stores the translated phrase
    std::string translated_;
    ///@brief If
    void useDefault(const std::string& phrase);
public:
    /**
     * @brief Where the magic happens. Checks a phrase against the dictionary, and if it finds a match to the selected language, stores the translated phrase in translated_
     * @param phrase Phrase to be translated
     * @param lang The selected language
     */
    Translate(const std::string& phrase, const std::string& lang);
    Translate();
    /// @brief This operator allows the constructor to be used directly in place of a string.
    operator std::string() const;

    /// @brief Static function that returns all languages present in dictionary.json
    static std::vector<std::string> getLangs();
};

#endif // TRANSLATE_H
