#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define CHAR_DEVICE_NODE "/dev/command"
#define BUF_SIZE 256

//线程1
#define COMMAND_a 'a' //开始
#define COMMAND_b 'b' //暂停
#define COMMAND_c 'c' //继续
#define COMMAND_d 'd' //重新开始
#define COMMAND_e 'e' //停止

//线程2
#define COMMAND_A 'A'
#define COMMAND_B 'B'
#define COMMAND_C 'C'
#define COMMAND_D 'D'
#define COMMAND_E 'E'

//所有线程
#define COMMAND_1 '1'
#define COMMAND_2 '2'
#define COMMAND_3 '3'
#define COMMAND_4 '4'
#define COMMAND_0 '0'

int Is_Check(char s){
	if(((s-'0') >= 0) && ((s-'0' <= 4)))
		return 1;
	else if((s >= 'a') && (s <= 'e'))
		return 1;
	else if((s >= 'A') && (s <= 'E'))
		return 1;
	else if(s == 'q' || s == 'Q')
		return 1;
	else
		return 0;
}

int main(void){
	int fd;
	int retval = 0;
	char data_buf[BUF_SIZE] = {0};

	/* open */
	fd = open(CHAR_DEVICE_NODE, O_RDWR);
	if(fd < 0){
		printf("open %s failed!\n", CHAR_DEVICE_NODE);
		return -1;
	}
	printf("open %s successfully!\n", CHAR_DEVICE_NODE);

	/* read */
	retval = read(fd, data_buf, 16);
	if(retval < 0){
		printf("read failed!\n");
		return -1;
	}
	printf("read %s successfully !\n", CHAR_DEVICE_NODE);

	/* write */
	retval = write(fd, "command - 1\n", 13);
	if(!retval < 0){
		printf("write failed!\n");
		return -1;
	}
	printf("write %s successfully!\n", CHAR_DEVICE_NODE);

	/* lseek */
	retval = lseek(fd, 0, 0);
	if(retval < 0){
		printf("lseek failed!\n");
		return -1;
	}
	printf("lessk %s successfully!\n", CHAR_DEVICE_NODE);

	/*ioctl */
	char i;
	printf("控制线程  1  :a:开始, b:暂停, c:继续, d:重新开始, e:停止\n");
	printf("控制线程  2  :A:开始, B:暂停, C:继续, D:重新开始, E:停止\n");
	printf("控制所有线程 :1:开始, 2:暂停, 3:继续, 4:重新开始, 0:停止\n");
	printf("按 q 或 Q 退出\n");
	while(1){
		printf("请输入命令:");
		scanf("%s", &i);
		if(!Is_Check(i)){
			printf("未找到命令，请重新输入\n");
			continue;
		}
		if(i == 'q' || i == 'Q')
			break;
		retval = ioctl(fd, i, 0);
		/*if(i == 1){
			
			retval = ioctl(fd, COMMAND_A, 0);
		}
		if(i == 2){
			retval = ioctl(fd, COMMAND_B, 0);
		}*/
		if(retval < 0){
			printf("ioctl failed!\n");
			return -1;
		}
		printf("ioctl %s successfully (COMMAND) i = %d!\n", CHAR_DEVICE_NODE, i);
	}
	/* close */
	close(fd);
	return 0;
}

