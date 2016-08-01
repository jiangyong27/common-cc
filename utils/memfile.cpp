// Copyright (c) 2013, Tencent Inc.
// All rights reserved.
//
// Author:jason  <jasonyjiang@tencent.com>
// Created: 2013-08-24

#include "memfile.h"

/**创建一个内存文件*/
MemFile::MemFile()
:m_base(new block_t)
,m_current_read(m_base)
,m_current_write(m_base)
,m_current_write_nr(0)
,m_read_ptr(0)
,m_write_ptr(0)
,m_b_read_caused_eof(false)
{
}


/** 通过文件加载一个内存文件*/
MemFile::MemFile(const std::string& path)
:m_base(new block_t)
,m_current_read(NULL)
,m_current_write(NULL)
,m_current_write_nr(0)
,m_read_ptr(0)
,m_write_ptr(0)
,m_b_read_caused_eof(false)
{
	Load(path);
}

/**释放内存文件 全部*/
MemFile::~MemFile()
{
    MemFree();
    if(m_base){delete m_base;m_base=NULL;}
}

/**释放内存文件 不释放头文*/
void MemFile::MemFree(void)
{
    if(!m_base)
        return ;
    block_t *h = m_base->next;
	while (h)
	{
		block_t *p = h;
		h = h -> next;
		delete p;
	}
}


/** 加载文件到内存*/
bool MemFile::Load(const std::string& path)
{
    ResetWrite();
	FILE *fil = ::fopen(path.c_str(),"r");

	if(!fil)
		return false;

	m_current_read = m_base;
	m_current_write = m_base;
	char slask[BLOCKSIZE];
	size_t n;
	n = ::fread(slask, 1, BLOCKSIZE, fil);
	while (n > 0)
	{
		Fwrite(slask, 1, n);
		n = ::fread(slask, 1, BLOCKSIZE, fil);
	}
	::fclose(fil);
	return true;
}

/**保存文件到磁盘*/
bool MemFile::Save(const std::string& path)
{
	FILE *fil = ::fopen(path.c_str(),"w");
	if(!fil) return false;

	char slask[BLOCKSIZE];
	size_t n;
	ResetRead();
	n = Fread(slask,1,BLOCKSIZE);
	while(n != 0)
	{
		::fwrite(slask,1,n,fil);
		n = Fread(slask,1,BLOCKSIZE);
	}

	::fclose(fil);
	return true;
}

/**读取数据从内存系统*/
size_t MemFile::Fread(char *ptr, size_t size, size_t nmemb) const
{
	size_t p = m_read_ptr % BLOCKSIZE;
	size_t sz = size * nmemb;
	size_t available = m_write_ptr - m_read_ptr;
	if (sz > available) // read beyond eof
	{
		sz = available;
		m_b_read_caused_eof = true;
	}
	if (!sz)
	{
		return 0;
	}
	if (p + sz < BLOCKSIZE)
	{
		memcpy(ptr, m_current_read -> data + p, sz);
		m_read_ptr += sz;
	}
	else
	{
		size_t sz1 = BLOCKSIZE - p;
		size_t sz2 = sz - sz1;
		memcpy(ptr, m_current_read -> data + p, sz1);
		m_read_ptr += sz1;
		while (sz2 > BLOCKSIZE)
		{
			if (m_current_read -> next)
			{
				m_current_read = m_current_read -> next;
				memcpy(ptr + sz1, m_current_read -> data, BLOCKSIZE);
				m_read_ptr += BLOCKSIZE;
				sz1 += BLOCKSIZE;
				sz2 -= BLOCKSIZE;
			}
			else
			{
				return sz1;
			}
		}
		if (m_current_read -> next)
		{
			m_current_read = m_current_read -> next;
			memcpy(ptr + sz1, m_current_read -> data, sz2);
			m_read_ptr += sz2;
		}
		else
		{
			return sz1;
		}
	}
	return sz;
}

/**写数据到内存文件*/
size_t MemFile::Fwrite(const char *ptr, size_t size, size_t nmemb)
{
	size_t p = m_write_ptr % BLOCKSIZE;
	int nr = (int)m_write_ptr / BLOCKSIZE;
	size_t sz = size * nmemb;
	if (m_current_write_nr < nr)
	{
		block_t *next = new block_t;
		m_current_write -> next = next;
		m_current_write = next;
		m_current_write_nr++;
	}
	if (p + sz <= BLOCKSIZE)
	{
		memcpy(m_current_write -> data + p, ptr, sz);
		m_write_ptr += sz;
	}
	else
	{
		size_t sz1 = BLOCKSIZE - p; // size left
		size_t sz2 = sz - sz1;
		memcpy(m_current_write -> data + p, ptr, sz1);
		m_write_ptr += sz1;
		while (sz2 > BLOCKSIZE)
		{
			if (m_current_write -> next)
			{
				m_current_write = m_current_write -> next;
				m_current_write_nr++;
			}
			else
			{
				block_t *next = new block_t;
				m_current_write -> next = next;
				m_current_write = next;
				m_current_write_nr++;
			}
			memcpy(m_current_write -> data, ptr + sz1, BLOCKSIZE);
			m_write_ptr += BLOCKSIZE;
			sz1 += BLOCKSIZE;
			sz2 -= BLOCKSIZE;
		}
		if (m_current_write -> next)
		{
			m_current_write = m_current_write -> next;
			m_current_write_nr++;
		}
		else
		{
			block_t *next = new block_t;
			m_current_write -> next = next;
			m_current_write = next;
			m_current_write_nr++;
		}
		memcpy(m_current_write -> data, ptr + sz1, sz2);
		m_write_ptr += sz2;
	}
	return sz;
}


/**读取一个字符串*/
char *MemFile::Fgets(char *s, int size) const
{
	int n = 0;
	while (n < size - 1 && !Eof())
	{
		char c;
		size_t sz = Fread(&c, 1, 1);
		if (sz)
		{
			if (c == 10)
			{
				s[n] = 0;
				return s;
			}
			s[n++] = c;
		}
	}
	s[n] = 0;
	return s;
}

/**格式化输出*/
void MemFile::Fprintf(const char *format, ...)
{
	va_list ap;
	char tmp[BLOCKSIZE];
	va_start(ap, format);
	vsnprintf(tmp, sizeof(tmp), format, ap);
	va_end(ap);
	Fwrite(tmp, 1, strlen(tmp));
}

/**返回内出文件大小*/
off_t MemFile::Size() const
{
	return (off_t)m_write_ptr;
}


/**是否读到文件尾部*/
bool MemFile::Eof() const
{
	return m_b_read_caused_eof; //(m_read_ptr < m_write_ptr) ? false : true;
}

/**重置读位置到文件头部*/
void MemFile::ResetRead() const
{
	m_read_ptr = 0;
	m_current_read = m_base;
}

/**清空文件*/
void MemFile::ResetWrite()
{
	m_write_ptr = 0;
	m_current_write = m_base;
	m_current_write_nr = 0;
}





