/**
 * Hi so this file should have everything tiger ftp needs
 * Yeah
 */

#ifndef __TIGER_FTP__
#define __TIGER_FTP__

//testing  only
//#define MAX_CLIENTS	2

#define	MAX_CLIENTS	100

#define TRANS_BUFF_SIZE (BUFF_SIZE + HEADER_SIZE)
#define HEADER_SIZE (sizeof(uint16_t))*2
#define BUFF_SIZE 8192 
#define PATH_LENGTH	256

#define FTP_CNT	0xFF3C
#define FTP_GET	0xFF2D
#define FTP_PUT  0xFF1E
#define FTP_PUT2 0xEE1E
#define FTP_LST  0xFF4B
#define FTP_CWD	0xFF5A
#define T_DISCONNECT	0x5AA5

#define CNT_SUCCESS	0xEE69
#define	FTP_FAIL	0xEE78

//idk I just came up with this ramdomly
#define DELIMITER	'\r'

enum rqst_type {CNT, GET, PUT, LST, CWD};

typedef struct ftp_msg{
	uint16_t type;
	uint16_t length;
	char	*msg;
} ftp_msg_t;

typedef struct ftp_put_msg{
	uint16_t filename_len;
	uint16_t file_len;
	char	*filename;
	char	*content;
} ftp_put_msg_t;

typedef struct client_record{
	char	*usrname;
	char	*password;
} client_record_t;

// the working context of each client
typedef struct client_dir {
	int 	client_fd;
	char	*dir;	
} client_dir_t;

// log-in authentication
// returning 1 on success
int authenticate (char *usrname, char *password);

// returning the client's current working directory
char *get_context (int client_fd);

//updating the working directory
int update_context (int client_fd, char *new_dir);

// list
int get_list(int client_fd, char *dst);

#endif

