#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <utility>
#include <filesystem>
#include <codecvt>
#include <coroutine>
#include <experimental/generator>

class RawWikiPage
{
public:
    explicit RawWikiPage(std::string contents) :
        m_contents(std::move(contents)),
        m_id(extractTag(m_contents, 0, "<id>", "</id>"))
    {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
        std::string title = extractTag(m_contents, 0, "<title>", "</title>");
        m_title = convert.from_bytes(title);
    }

    std::string contents()
    {
        return m_contents;
    }

    std::u32string title()
    {
        return m_title;
    }

    std::string id()
    {
        return m_id;
    }

private:
    const std::string m_contents;
    std::u32string m_title;
    const std::string m_id;

    static std::string extractTag(
        const std::string &input,
        int offset,
        const std::string &openTag,
        const std::string &closeTag)
    {
        size_t openOffset = input.find(openTag, offset) + openTag.length();
        size_t closeOffset = input.find(closeTag, openOffset);
        return input.substr(openOffset, closeOffset - openOffset);
    }
};

std::experimental::generator<int> test()
{
    co_yield 3;
}

class WikiDump
{
public:
    explicit WikiDump(const std::string &filename) :
        instream(filename)
    {
    }

    RawWikiPage readPage()
    {
        std::string line;
        std::stringstream contents;
        bool started = false;
        while (std::getline(instream, line))
        {
            if (!started && line.find("<page>") != std::string::npos)
            {
                started = true;
            }
            if (started)
            {
                contents << line << "\n";
                if (line.find("</page>") != std::string::npos)
                {
                    break;
                }
            }
        }
        return RawWikiPage(contents.str());
    }

    [[nodiscard]] bool eof() const
    {
        return instream.eof();
    }

    void writeAllToDir(const std::filesystem::path &dirpath)
    {
//        int counter = 0;
        while (!eof())
        {
            auto page = readPage();
//            std::cout << page.title() << "\n";
            try
            {
                auto subdir = page.title().substr(0, 3);
                std::filesystem::path newpath = dirpath / subdir;
                try
                {
                    // some names are verboten in Windows, like "CON"
                    std::filesystem::exists(newpath);
                }
                catch (std::filesystem::filesystem_error &err)
                {
                    newpath = dirpath / "other";
                }
                if (!std::filesystem::exists(newpath))
                {
//                    std::cout << "Creating directory " << newpath << "\n";
                    std::filesystem::create_directory(newpath);
                }
                std::ofstream outstream(newpath / (page.title() + U".xml"));
                outstream << page.contents();
                outstream.close();
            }
            catch (std::filesystem::filesystem_error &err)
            {
                std::cout << "File system error: " << err.what() << "\n";
            }
//            if (++counter > 1000)
//            {
//                break;
//            }
        }
    }

private:
    std::ifstream instream;
};

int main()
{
    std::string filename(
        "F:/wiki/enwiki-20210120-pages-articles-multistream.xml"
    );
    WikiDump dump = WikiDump(filename);
    dump.writeAllToDir("F:/wiki/sorted2");
    return 0;
}
