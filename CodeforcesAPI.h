//
// Created by xtdx on 2023/4/25.
//

#ifndef PROJECT_CODEFORCESAPI_H
#define PROJECT_CODEFORCESAPI_H

#include <bits/stdc++.h>
#include <curl/curl.h>
#include "cJSON/cJSON.h"

class CodeforcesAPI {
public:
    std::string Username;
    static std::string Get(const std::string& User);
    static void PrintData();
private:
    static std::string request(const std::string& url);
    static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* data);
    static void parseProblems(std::string& data);
};

#endif //PROJECT_CODEFORCESAPI_H
