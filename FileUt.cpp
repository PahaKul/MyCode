#include <iostream>
#include <fstream>
#include <array>
#include <memory>

std::string FileUtility::ReadFromFile(const std::string& fileName)
{
    std::ifstream file;
    std::string result = "";
    try
    {
        file.open(fileName.c_str());
        if (file.is_open())
        {
            file.seekg(0, std::ios::end);
            const auto size = file.tellg();
            std::unique_ptr<char[]> buffer(new char[size]);
            file.seekg(0, std::ios::beg);
            file.read(buffer.get(), size);
            result.assign(buffer.get());
            file.close();
        }
        else
        {
            LOG_ERROR("Error open file: " + fileName + " for read.");
        }
    }
    catch (std::exception &e)
    {
        LOG_ERROR("ReadFile error: " + static_cast<std::string>(e.what()));
    }
    catch (...)
    {
        LOG_ERROR("Read file error: Unknown error");
    }
    return result;
}


bool FileUtility::WriteToFile(const std::string& fileName, const std::string dataForWrite)
{
    std::ofstream file;
    bool result = false;
    try
    {
        file.open(fileName.c_str());
        if (file.is_open())
        {
            file << dataForWrite;
            file.close();
            result = true;
        }
        else
        {
            LOG_ERROR("Error open file: " + fileName + " for write.");
        }
    }
    catch (std::exception &e)
    {
        LOG_ERROR("WriteFile error: " + static_cast<std::string>(e.what()));
    }
    catch (...)
    {
        LOG_ERROR("Write file error: Unknown error");
    }

    return result;
}
