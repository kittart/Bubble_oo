#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <thread> 
#include "block_queue.h"
#include "locker.h" // 确保 locker.h 被包含
#include <fstream>

using namespace std;

// 日志级别枚举
enum LogLevel {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR
};

// 存储解析日志文件的结构体
struct ParsedLog {
    std::string timestamp; // 时间戳
    std::string device_id; // 设备ID
    std::string level;     // 日志级别
    std::string content;   // 日志内容
};

class Log
{
public:
    // 获取单例实例
    static Log *get_instance()
    {
        static Log instance;
        return &instance;
    }

    // 刷新日志线程
    void flush_log_thread(); // 修改为非静态成员函数

    // 初始化日志系统，参数包括文件名、是否关闭日志、缓冲区大小、最大行数和最大队列大小
    bool Log::init(const char *file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size);

    
    // 写入日志
    void write_log(int level, const char *format, ...);
    
    // 刷新日志文件
    void flush(void);

    //接收原始的要公共类
    void receiveLog(const std::string& raw_log); // 接收原始日志

private:
    Log(); // 构造函数
    virtual ~Log(); // 析构函数

    void async_write_log(); // 异步写入日志
    bool check_log_size(); // 检查日志文件大小
    void rotate_logs(); // 轮换日志文件
    
    ParsedLog parseLog(const std::string& log); // 解析日志
    bool init(const std::string &file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size);
    void queryLogs(const std::string &device_id); // 查询日志

    const char* log_level_to_string(int level); // 将日志级别转换为字符串
    int levelToInt(const std::string &level);

private:
    char dir_name[128]; // 日志文件目录
    char log_name[128]; // 日志文件名
    int m_split_lines;  // 日志最大行数
    int m_log_buf_size; // 日志缓冲区大小
    long long m_count;  // 日志行数记录
    int m_today;        // 记录当前时间
    std::ofstream m_fp; // 将 FILE * m_fp 更改为 std::ofstream m_fp
    char *m_buf;        // 日志缓冲区
    block_queue<string> *m_log_queue; // 阻塞队列
    bool m_is_async;                  // 是否异步标志位
    locker m_mutex;                   // 互斥锁
    int m_close_log; // 关闭日志的标志
    long max_size = 10 * 1024 * 1024; // 10MB
    //测试文件里面的
    int m_close_log; // 确保在这里定义
};

// 宏定义用于不同级别的日志记录
#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(DEBUG, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(INFO, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(WARN, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(ERROR, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif
