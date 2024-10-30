#include <string.h>
#include <time.h>
#include <chrono>
#include <ctime>
#include <stdarg.h>
#include "log.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>

using namespace std;

// 构造函数
Log::Log() {
    m_count = 0; // 初始化日志行数
    m_is_async = false; // 初始化为同步
}

// 析构函数
Log::~Log() {
    if (m_fp.is_open()) {
        m_fp.close(); // 关闭日志文件
    }
}

// 接收原始日志并解析
void Log::receiveLog(const std::string &raw_log) {
    ParsedLog parsed_log = parseLog(raw_log);
    write_log(levelToInt(parsed_log.level), "%s [%s] %s", parsed_log.timestamp.c_str(), parsed_log.device_id.c_str(), parsed_log.content.c_str());
}

// 解析日志字符串
ParsedLog Log::parseLog(const std::string &log) {
    ParsedLog parsed_log;
    std::istringstream iss(log);
    std::string level;

    // 读取时间戳、设备ID和日志级别
    iss >> parsed_log.timestamp >> parsed_log.device_id >> level;
    std::getline(iss, parsed_log.content);
    parsed_log.level = level; // 设置解析后的级别

    return parsed_log;
}

// 初始化日志系统
bool Log::init(const std::string &file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size) {
    if (max_queue_size >= 1) {
        m_is_async = true; // 启用异步写入
        m_log_queue = new block_queue<string>(max_queue_size); // 创建阻塞队列
        std::thread(&Log::flush_log_thread, this).detach(); // 创建刷新日志线程
    }

    m_close_log = close_log; // 设置关闭日志标志
    m_log_buf_size = log_buf_size; // 设置缓冲区大小
    m_buf = new char[m_log_buf_size]; // 分配缓冲区
    memset(m_buf, '\0', m_log_buf_size); // 清空缓冲区
    m_split_lines = split_lines; // 设置最大行数

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    std::string log_full_name;

    // 根据文件路径和当前日期生成完整日志文件名
    if (file_name.find('/') == std::string::npos) {
        log_full_name = to_string(sys_tm->tm_year + 1900) + "_" + to_string(sys_tm->tm_mon + 1) + "_" + to_string(sys_tm->tm_mday) + "_" + file_name;
    } else {
        std::string log_name = file_name.substr(file_name.find_last_of('/') + 1);
        std::string dir_name = file_name.substr(0, file_name.find_last_of('/') + 1);

        log_full_name = dir_name + to_string(sys_tm->tm_year + 1900) + "_" + to_string(sys_tm->tm_mon + 1) + "_" + to_string(sys_tm->tm_mday) + "_" + log_name;
    }

    m_today = sys_tm->tm_mday; // 记录今天的日期
    m_fp.open(log_full_name, std::ios::out | std::ios::app); // 打开日志文件
    if (!m_fp.is_open()) {
        return false; // 打开失败
    }

    return true; // 初始化成功
}

#include <chrono>

void Log::write_log(int level, const char *format, ...) {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    struct tm *sys_tm = localtime(&now_time_t);
    
    std::string s;

    // 根据日志级别设置前缀
    switch (level) {
        case DEBUG: s = "[debug]: "; break;
        case INFO: s = "[info]: "; break;
        case WARN: s = "[warn]: "; break;
        case ERROR: s = "[error]: "; break;
        default: s = "[info]: "; break;
    }

    m_mutex.lock(); // 锁定互斥体
    m_count++; // 增加日志计数

    // 检查是否需要轮换日志
    if (m_today != sys_tm->tm_mday || m_count % m_split_lines == 0) {
        // ... (日志轮换代码)
    }

    // 写入日志内容
    va_list valist;
    va_start(valist, format);
    vsnprintf(m_buf, m_log_buf_size, format, valist); // 写入日志内容

    m_fp << s << m_buf << std::endl; // 使用ofstream写入文件

    m_mutex.unlock(); // 解锁互斥体

    // 异步写入日志
    if (m_is_async && !m_log_queue->full()) {
        m_log_queue->push(m_buf); // 将日志推入队列
    }
}

// 刷新日志文件
void Log::flush(void) {
    m_mutex.lock(); // 锁定互斥体
    m_fp.flush(); // 刷新文件
    m_mutex.unlock(); // 解锁互斥体
}

// 刷新日志线程
void Log::flush_log_thread() {
    while (m_is_async) {
        // 从队列中获取日志并写入
        string log;
        if (m_log_queue->pop(log)) {
            write_log(INFO, "%s", log.c_str());
        }
    }
}

// 日志级别转换为整数
int Log::levelToInt(const std::string &level) {
    if (level == "[debug]:") return DEBUG;
    if (level == "[info]:") return INFO;
    if (level == "[warn]:") return WARN;
    if (level == "[error]:") return ERROR;
    return INFO; // 默认返回INFO
}
