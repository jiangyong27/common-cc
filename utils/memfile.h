// Copyright (c) 2013, Tencent Inc.
// All rights reserved.
//
// Author:jason  <jasonyjiang@tencent.com>
// Created: 2013-08-24
//

#ifndef JASONYJIANG_MEMFILE_H
#define JASONYJIANG_MEMFILE_H

#include <stdio.h>
#include <stdarg.h>

#include <map>
#include <cstring>
#include <iostream>
#include <fstream>


#define BLOCKSIZE 32768

/**内存文件系统*/
class MemFile
{
public:
	/** 内存文件块*/
	struct block_t {
		block_t() : next(NULL) {}
		struct block_t *next;
		char data[BLOCKSIZE];
	};
public:
	MemFile();

	MemFile(const std::string& path);
	~MemFile();

	//bool fopen(const std::string& path, const std::string& mode);
    void Close() const;

    /**释放内存*/
    void MemFree();

    /**标准读取和写入*/
	size_t Fread(char *ptr, size_t size, size_t nmemb) const;
	size_t Fwrite(const char *ptr, size_t size, size_t nmemb);

    /**读取字符串*/
	char *Fgets(char *s, int size) const;

    /**格式化写入*/
	void Fprintf(const char *format, ...);

    /**获得文件大小*/
	off_t Size() const;

    /**检测文件是否结尾*/
	bool Eof() const;

    /**重置读写位置*/
	void ResetRead() const;
	void ResetWrite();

	//jasonyjiang
    /**保存内存文件到磁盘*/
	bool Save(const std::string& path);

    /**读取磁盘文件到内存文件*/
	bool Load(const std::string& path);

private:
    /**禁用拷贝够着函数*/
    MemFile(MemFile&){}

    /**禁用赋值构造函数*/
	MemFile& operator=(const MemFile& ) { return *this; } // assignment operator

	block_t *m_base;
	mutable block_t *m_current_read;
	block_t *m_current_write;
	int m_current_write_nr;
	mutable size_t m_read_ptr;
	size_t m_write_ptr;
	mutable bool m_b_read_caused_eof;
};



#endif // MemFile_H

