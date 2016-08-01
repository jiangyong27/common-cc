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

/**�ڴ��ļ�ϵͳ*/
class MemFile
{
public:
	/** �ڴ��ļ���*/
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

    /**�ͷ��ڴ�*/
    void MemFree();

    /**��׼��ȡ��д��*/
	size_t Fread(char *ptr, size_t size, size_t nmemb) const;
	size_t Fwrite(const char *ptr, size_t size, size_t nmemb);

    /**��ȡ�ַ���*/
	char *Fgets(char *s, int size) const;

    /**��ʽ��д��*/
	void Fprintf(const char *format, ...);

    /**����ļ���С*/
	off_t Size() const;

    /**����ļ��Ƿ��β*/
	bool Eof() const;

    /**���ö�дλ��*/
	void ResetRead() const;
	void ResetWrite();

	//jasonyjiang
    /**�����ڴ��ļ�������*/
	bool Save(const std::string& path);

    /**��ȡ�����ļ����ڴ��ļ�*/
	bool Load(const std::string& path);

private:
    /**���ÿ������ź���*/
    MemFile(MemFile&){}

    /**���ø�ֵ���캯��*/
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

