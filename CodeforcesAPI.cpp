#include "CodeforcesAPI.h"

std::vector<int> ProblemTime; // 每道题的做题时间
std::set<std::pair<std::string, int>> Problem; // ProblemID, OK次数
std::set<std::pair<std::string, int>> ProblemTags; // Tag, OK次数
std::set<std::pair<int, int>> ProblemRating; // 每道题的难度, 通过次数
std::set<std::pair<std::string, int>> ProblemVerdict; // 统计verdict各种结果次数
std::set<std::pair<std::string, int>> ProblemLanguage; // 统计语言各种结果次数
std::set<std::pair<std::string, int>> ProblemIndex; // 统计题目的index
unsigned long long ProblemCount = 0;


size_t CodeforcesAPI::WriteCallback(void* ptr, size_t size, size_t nmemb, void* data) {
    ((std::string*)data)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

std::string CodeforcesAPI::request(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    // 取消ssl认证，不然收不到信息
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        res = curl_easy_perform(curl); // 在这里会发起请求，在这之后所有操作才奏效

        if (res != CURLE_OK) std::cerr << "ERROR: " << curl_easy_strerror(res) << std::endl;

        curl_easy_cleanup(curl);
    }
    else std::cerr << "INIT FAIL" << std::endl;

    curl_global_cleanup();

    return readBuffer;
}

std::string CodeforcesAPI::Get(const std::string& User) {
    std::string temp = request("https://codeforces.com/api/user.status?handle=" + User + "&from=1&count=1000000000");
    parseProblems(temp);
    return temp;
}

void CodeforcesAPI::parseProblems(std::string& data) {
    cJSON* root = cJSON_Parse(data.c_str());
    // 解析数据 找出verdict是 OK 的题目
    cJSON* result = cJSON_GetObjectItem(root, "result");
    int size = cJSON_GetArraySize(result);
    for (int i = 0; i < size; i++) {
        // 先找到problem
        cJSON* problem = cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "problem");



        // 找到verdict，添加到ProblemVerdict里面
        std::string verdict = cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "verdict")->valuestring;
        // ProblemVerdict里面的元素是一个pair，第一个是verdict，第二个是verdict的次数
        // 如果ProblemVerdict里面没有这个verdict的话就新建一个 如果有的话就把verdict的次数加一, 用find_if
        auto it_verdict = std::find_if(ProblemVerdict.begin(), ProblemVerdict.end(),
                                       [&](const std::pair<std::string, int>& elem){
                       return elem.first == verdict;
                   });
        if (it_verdict != ProblemVerdict.end()) {
            int count = it_verdict->second;
            ProblemVerdict.erase(it_verdict);
            ProblemVerdict.emplace(verdict, count + 1);
        } else ProblemVerdict.emplace(verdict, 1);



        // 找到语言，添加到ProblemLanguage里面
        std::string language = cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "programmingLanguage")->valuestring;
        // ProblemLanguage里面的元素是一个pair，第一个是language，第二个是language的次数
        // 如果ProblemLanguage里面没有这个language的话就新建一个 如果有的话就把language的次数加一
        auto it_language = std::find_if(ProblemLanguage.begin(), ProblemLanguage.end(),
                                        [&](const std::pair<std::string, int>& elem){
                        return elem.first == language;
                    });
        if (it_language != ProblemLanguage.end()) {
            int count = it_language->second;
            ProblemLanguage.erase(it_language);
            ProblemLanguage.emplace(language, count + 1);
        } else ProblemLanguage.emplace(language, 1);



        // 如果verdict是OK的话就把ProblemID加入到已经做过的题目里面 并且记录OK题目的次数
        if (verdict == "OK") {
            // 在problem里面找到contestId和index 合并起来作为ProblemID
            std::string index = cJSON_GetObjectItem(problem, "index")->valuestring;
            std::string problemid = std::to_string(cJSON_GetObjectItem(problem, problem->child->string)->valueint) + index;
            // Problem里面的元素是一个pair，第一个是ProblemID，第二个是OK的次数
            // 如果Problem里面没有这个ProblemID的话就新建一个 如果有的话就把OK的次数加一
            auto it_Problem = std::find_if(Problem.begin(), Problem.end(),
                                           [&](const std::pair<std::string, int>& elem){
                           return elem.first == problemid;
                       });
            if (it_Problem != Problem.end()) {
                int count = it_Problem->second;
                Problem.erase(it_Problem);
                Problem.emplace(problemid, count + 1);
            } else Problem.emplace(problemid, 1);
            // 再记录index的次数，没有就新建，有就加一
            auto it_Index = std::find_if(ProblemIndex.begin(), ProblemIndex.end(),
                                         [&](const std::pair<std::string, int>& elem){
                         return elem.first == index;
                     });
            if (it_Index != ProblemIndex.end()) {
                int count = it_Index->second;
                ProblemIndex.erase(it_Index);
                ProblemIndex.emplace(index, count + 1);
            } else ProblemIndex.emplace(index, 1);



            /* 以下是对Tags的处理 */
            // 在problem里面找到tags，是一个数组，把它添加到vector里面
            cJSON* tags = cJSON_GetObjectItem(problem, "tags");
            std::vector<std::string> Tags;
            int tags_size = cJSON_GetArraySize(tags);
            for (int j = 0; j < tags_size; j++) {
                Tags.emplace_back(cJSON_GetArrayItem(tags, j)->valuestring);
            }
            // 把Tags里面的元素添加到ProblemTags里面，如果已经有了就把次数加一
            for (auto& tag : Tags) {
                auto it_Tag = std::find_if(ProblemTags.begin(), ProblemTags.end(),
                                           [&](const std::pair<std::string, int>& elem){
                               return elem.first == tag;
                           });
                if (it_Tag != ProblemTags.end()) {
                    int count = it_Tag->second;
                    ProblemTags.erase(it_Tag);
                    ProblemTags.emplace(tag, count + 1);
                } else ProblemTags.emplace(tag, 1);
            }
            /* 以上是对Tags的处理 */



            // 找到提交时间
            int time = cJSON_GetObjectItem(cJSON_GetArrayItem(result, i), "creationTimeSeconds")->valueint;
            ProblemTime.emplace_back(time);



            // 找到题目难度, 如果没有就跳过，如果有就把难度和通过次数加入到ProblemRating里面，如果已经有了就把通过次数加一
            cJSON* rating = cJSON_GetObjectItem(problem, "rating");
            if (rating == nullptr) continue;
            int Rating = rating->valueint;
            auto it_Rating = std::find_if(ProblemRating.begin(), ProblemRating.end(),
                                          [&](const std::pair<int, int>& elem){
                          return elem.first == Rating;
                      });
            if (it_Rating != ProblemRating.end()) {
                int count = it_Rating->second;
                ProblemRating.erase(it_Rating);
                ProblemRating.emplace(Rating, count + 1);
            } else ProblemRating.emplace(Rating, 1);
        }
    }
    ProblemCount = Problem.size();
}

void CodeforcesAPI::PrintData() {
    // 把所有的数据都打印出来
    std::cout << "ProblemID: " << std::endl;
    for (const auto & it : Problem) {
        std::cout << it.first << " " << it.second << std::endl;
    }
    std::cout << std::endl;

    std::cout << "ProblemTags: " << std::endl;
    for (const auto & ProblemTag : ProblemTags) {
        std::cout << ProblemTag.first << " " << ProblemTag.second << std::endl;
    }
    std::cout << std::endl;

    std::cout << "ProblemTime: " << std::endl;
    for (int & it : ProblemTime) {
        std::cout << it << std::endl;
    }
    std::cout << std::endl;

    std::cout << "ProblemRating: " << std::endl;
    for (const auto & it : ProblemRating) {
        std::cout << it.first << " " << it.second << std::endl;
    }
    std::cout << std::endl;

    std::cout << "ProblemVerdict: " << std::endl;
    for (const auto & it : ProblemVerdict) {
        std::cout << it.first << " " << it.second << std::endl;
    }
    std::cout << std::endl;

    std::cout << "ProblemLanguage: " << std::endl;
    for (const auto & it : ProblemLanguage) {
        std::cout << it.first << " " << it.second << std::endl;
    }
    std::cout << std::endl;

    std::cout << "ProblemIndex: " << std::endl;
    for (const auto & it : ProblemIndex) {
        std::cout << it.first << " " << it.second << std::endl;
    }
    std::cout << std::endl;

    std::cout << "ProblemCount: " << std::endl;
    std::cout << ProblemCount << std::endl;
}
