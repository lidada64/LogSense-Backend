#include <iostream>
#include <fstream>  // 文件流库
#include <string>
#include <thread>   // 用于 sleep
#include <chrono>   // 用于时间单位
#include<nlohmann/json.hpp>
#include<cpr/cpr.h>
#include<cstdlib>

// 你的日志文件路径 (跟 log_generator.sh 生成的一致)
const std::string LOG_FILE_PATH = "../app.log";//注意文件路径问题，app.log在build上一级文件夹里

const std::string MODEL_ENDPOINT = "https://dashscope-intl.aliyuncs.com/api/v1/services/aigc/text-generation/generation";
// 使用的模型版本 
const std::string MODEL_NAME = "qwen-flash";


void analyzelog(const std::string& text, const std::string& apiKey){
    nlohmann::json payload = {
    {"model", MODEL_NAME}, // 1. 指定你要找哪位
    {"input", {
        {"messages", {     // 2. 对话历史（Context）
            // --- 第一句：系统设定 (System Prompt) ---
            {
                {"role", "system"}, 
                {"content", "你是一名资深的系统运维专家,你需要阅读并分析传入的unix日志,并按照:\"严重程度(高/中/低)+是什么+解决方案的格式输出,不超过30个字,切中肯綮\"'"} // 也就是给 AI 洗脑，设定人设
            },
            // --- 第二句：用户的提问 (User Prompt) ---
            {
                {"role", "user"},
                {"content", text} // 这里放入我们要查的那行日志
            }
        }}
    }},
    {"parameters", {
            {"result_format", "message"} // 强制返回 message 格式，方便解析
        }} // 3. 其他参数
};

std::cout << "📤 [Qwen] 发送中: " << text << " ..." << std::endl;
cpr::Response r=cpr::Post(

    cpr::Url{MODEL_ENDPOINT},//address
    cpr::Header{//sender
        {"Content-Type", "application/json"},   // 告诉对方：信封里装的是 JSON 数据，不是图片或纯文本
        {"Authorization", "Bearer " + apiKey}   // 【关键】这就是你的“通行证” (Bearer Token)

    },


    cpr::Body{payload.dump()} //序列化,把发送的内容变成字符串(Serialization)
);


if(r.status_code==200){//接收成功

try{
        nlohmann::json response = nlohmann::json::parse(r.text);//将字符串改为json

std::fstream jsonFile;

jsonFile.open("../reply.json",std::ios::app);

if (!jsonFile.is_open()) {
	std::cout << "fail to open" << std::endl;

	return;
}
jsonFile<<response<<std::endl;

jsonFile.close();



std::string analysis = response["output"]["choices"][0]["message"]["content"];



std::cout<<"AI analyze: "<<analysis<<std::endl;

}catch (const std::exception& e) 
{
std::cerr << "❌ JSON 解析失败: " << e.what() << "\n原始数据: " << r.text << std::endl;
}
}

else{
    std::cerr<<"连接失败,code= "<<r.status_code<<std::endl;
    std::cerr << "错误详情: " << r.text << std::endl;
    return;
}
  
}








int main() {



    //读取api key

    const char* env_p = std::getenv("QWEN_API_KEY");//从环境变量中获取api key
    //getenv返回指针,不接受c++写法
    if(!env_p){
        std::cout<<"Error:API key 未配置"<<std::endl;
        return 1;
    }

    std::string api_key=env_p;



    std::cout << "🔍 LogSense Agent 启动..." << std::endl;
    std::cout << "正在监听文件: " << LOG_FILE_PATH << std::endl;

    // 1. 打开文件
    std::ifstream file(LOG_FILE_PATH);

    // 等待文件创建 (防止生成器还没启动程序就崩了)
    while (!file.is_open()) {
        std::cout << "等待日志文件生成..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        file.open(LOG_FILE_PATH);
    }

    // 2. [关键点] 把光标移动到文件末尾 (Seek to End)
    // std::ios::end = 文件末尾
    file.seekg(0, std::ios::end);
    std::cout << "✅ 文件已打开，光标已移至末尾，开始监听新增内容..." << std::endl;

    std::string line;
    while (true) {
        // 尝试读取一行
        // std::getline 会尝试从当前光标读到换行符
        if (std::getline(file, line)) {
            // --- A. 如果读到了新行 ---
            std::cout << "------------------------------------------------" << std::endl;
            std::cout << "🆕 捕获新日志: " << line << std::endl;
            
            analyzelog(line,api_key);
            // TODO: 在这里调用 Gemini API (Phase 3 会把之前的代码加回来)
            // analyzeLog(line); 
        } 
        else {
            // --- B. 如果没读到 (到了文件末尾) ---
            
            // 这是一个 C++ 文件流的坑：
            // 一旦读到 EOF (End of File)，文件流会进入“报错状态”，拒绝继续工作。
            // 必须调用 .clear() 清除错误标志，才能继续读取未来写入的数据。
            if (file.eof()) {
                file.clear(); 
            }
            
            // 休息 1 秒，避免把 CPU 跑满 (Polling)轮询
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return 0;
}