/**
* 功能说明：
*     1.模拟http分块传输。只用于示例，切不可直接拷贝用于实际开发中。
*
* 修改历史：
*     1.2021-6-24 薛米样<xuemiyang@zlg.cn> 创建。
*/

#include <stdio.h>
#include "zlg_iot_http_client.h"

IoT_Error_t on_body(zlg_iot_http_client_t* c) {
	static unsigned char *p = NULL;
	static size_t size = 0;
	size_t tmp_size = c->size + size;
	unsigned char *tmp = malloc(tmp_size + 1);
	if (p != NULL) {
		memcpy(tmp, p, size);
	}
	memcpy(tmp + size, c->body, c->size);
	tmp[tmp_size] = '\0';
	free(p);
	p = tmp;
	size = tmp_size;
	if (c->finish) {
		printf("%s\n", p);
		free(p);
		p = NULL;
		tmp = NULL;
		size = 0;
	}

	return SUCCESS;
}

static int ch16toi(char ch) {
	if (ch <= '9' && ch >= '0')
		return ch - '0';
	if (ch <= 'f' && ch >= 'a')
		return ch - 'a' + 10;
	if (ch <= 'F' && ch >= 'A')
		return ch - 'A' + 10;
	return -1;
}

static int str16toi(char *str) {
	int n = 0;
	int i = 1;
	int result = ch16toi(*str);
	while (result != -1) {
		n = n * 16 + result;
		result = ch16toi(*(str + i));
		i++;
	}
	return n;
}

/**transfer_encoding chunked**/
size_t chunk_left = 0; // 防止没有传输完一个chunk，记录下次读取的不完整chunk大小
const size_t chunk_size_max_len = 16;
unsigned char chunk_size_buff[16] = { 0 }; // 处理chunk size不完整的情况 1 a\r\n, 需要拼接成1a\r\n，这里认为chunk size的长度最大为16
size_t chunk_size_buff_len = 0;

void check_chunked_transfer(zlg_iot_http_client_t *c, unsigned char *body, size_t size, size_t offset) {
	c->body = body;
	c->size = size;
	c->offset = offset;
	c->body[size] = '\0';
	c->finish = false;
	/**
	* 1a\r\n
	* chunk内容
	* \r\n
	* 1c\r\n
	* chunk内容
	* \r\n
	* 0\r\n
	* \r\n
	* */
	unsigned char *head = body;

	while (body - head < size) {
		size_t chunk_size = 0;
		if (chunk_left > 0) {
			chunk_size = chunk_left;
			chunk_left = 0;
		}
		else {
			if (chunk_size_max_len - chunk_size_buff_len < size - (body - head)) {
				memcpy(chunk_size_buff + chunk_size_buff_len, body, chunk_size_max_len - chunk_size_buff_len);
			}
			else {
				memcpy(chunk_size_buff + chunk_size_buff_len, body, size - (body - head));
			}
			unsigned char *chunk_size_buff_p = chunk_size_buff;
			if (chunk_size_buff[0] == '\r' && chunk_size_buff[1] == '\n') { // 跳过chunk结尾
				chunk_size_buff_p += 2;
			}
			unsigned char *p = strstr(chunk_size_buff_p, "\r\n");
			if (p == NULL) { // 没有获取完整的chunk size
				chunk_size_buff_len = chunk_size_buff_len + size - (body - head);
				break;
			}
			size_t chunk_size_offset = p + 2 - chunk_size_buff - chunk_size_buff_len;
			chunk_size = str16toi(chunk_size_buff_p);
			memset(chunk_size_buff, 0x00, chunk_size_max_len);
			chunk_size_buff_len = 0;
			body += chunk_size_offset;
		}
		c->size = chunk_size;
		c->offset = offset;
		c->body = body;
		offset += chunk_size;
		if (body - head + chunk_size > size) {//读取的数据不够一个chunk的情况
			chunk_left = body - head + chunk_size - size;
			if (body - head < size) {
				c->size = size - (body - head);
				offset = offset - chunk_size + c->size;
				c->on_body(c);
			}
			break;
		}
		body = body + chunk_size; // 先移动到下一个chunk，再截取上一个chunk的字符串。
		unsigned char *p = strstr(body, "\r\n");
		if (p == NULL) { // 移动不到chunk的结尾
			memcpy(chunk_size_buff + chunk_size_buff_len, body, size - (body - head));
			chunk_size_buff_len = chunk_size_buff_len + size - (body - head);
			body = head + size;
		}
		else {
			body = p + 2;
		}
		c->body[chunk_size] = '\0'; // 截取上一个chunk的字符串。
		c->finish = chunk_size == 0;
		c->on_body(c);
	}
}

int main(int argc, char *argv[]) {
  // 消除argv未引用的告警
  (void)(argc);
  (void)(argv);

	zlg_iot_http_client_t c;
	memset(&c, 0x00, sizeof(c));
	c.hostname = "120.79.119.10";
	c.protocol = "https";
	c.port = 443;
  char cookie[] = "jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyaWQiOiI1ZDRkMGNjYTllOGUxODcyNDA5ZjA4OTIiLCJyb2xlIjowLCJ1c2VybmFtZSI6InpsZzAwOCIsIm1vYmlsZSI6IjE1MDE4NzYwNDc5IiwiZGV2X2V4dHJhIjp7ImRhdGFfZXhwaXJlIjozMCwiZGF0YV9jYXBhY2l0eSI6MjA0OCwiZGF0YV9zaXplIjo5Ny4wOTc2NTYyNSwidXBkYXRlX3RpbWUiOjE2MjQ1MDM2MTAsImRldnR5cGVfY2FwYWNpdHkiOjEwLCJkZXZfY2FwYWNpdHkiOjE4LCJkZXZfY29ubmVjdF9jYXBhY2l0eSI6MTgsImRldl90cmFuc21pdF9jYXBhY2l0eSI6MTgsImRldl9leHBvcnRfZGF0YV9saW1pdCI6MTAwMDB9LCJpYXQiOjE2MjQ1MDY3MDAsImlzc3VlciI6Imh0dHBzOi8vMTIwLjc5LjExOS4xMCIsImV4cCI6MTYyNTM3MDcwMH0.VM2jD70BIYEEb0sxmTFO10y-4l1c5IGqkOWTcSwYURY";
	c.cookie = cookie;
	c.path = "/v1/devices";
	c.on_body = on_body;
	IoT_Error_t rc = zlg_iot_https_get(&c);
	printf("%d", rc);

	// "[{\"name\": \"jack\",\"age\": 23},{\"name\": \"rose\",\"age\": 18}]" 

	size_t offset = 0;

	/**
	* 1a\r\n
	* chunk内容
	* \r\n
	* 1f\r\n
	* chunk内容
	* \r\n
	* 0\r\n
	* \r\n
	* */
	// 处理chunk size 不完整情况
	unsigned char body[2048] = { 0 };
	const unsigned char *str = "19\r\n[{\"name\": \"jack\",\"age\": 2\r\n1e\r\n3},{\"name\": \"rose\",\"age\": 18}]\r\n0\r\n\r\n";
	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body, 1, 0); // 1

	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 1, 2, 1); // a\r

	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 3, 1, 3); // \n

	offset += 4;
	// 处理chunk不完善的情况 "19\r\n[{\"name\": \"jack\",\"age\": 2\r";
	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 4, 0x10, offset);

	offset += 0x10;
	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 4 + 0x10, 0xa, offset); // \r

	offset += 0xa;
	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 4 + 0x1a, 1, offset);  // \n

	offset += 1;
	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 4 + 0x1b, 4, offset);  // 1e\r\n

	offset += 4;
	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 10 + 0x19, 0x1e, offset); //没有chunk结束\r\n

	offset += 0x1f;
	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 10 + 0x19 + 0x1e, 2, offset); //\r\n

	offset += 2;
	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 12 + 0x19 + 0x1e, 2, offset); //0\r

	offset += 2;
	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 14 + 0x19 + 0x1e, 1, offset); //\n

	offset += 1;
	memcpy(body, str, strlen(str));
	check_chunked_transfer(&c, body + 15 + 0x19 + 0x1e, 2, offset); //\r\n
	return 0;
}