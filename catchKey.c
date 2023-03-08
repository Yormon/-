/*************************************************************************
  > File Name: catchKey.c
  > Author: yormon
  > Mail: 1547215764@qq.com 
  > Created Time: 2023年03月04日 星期六 15时21分45秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include <signal.h>

#define KEY1 "E CamX"
#define KEY2 "E CHIUSECASE"

char buffer_A[1024];
char buffer_B[1024];
pthread_mutex_t lock;
pthread_cond_t cdA, cdB, cdC;

int flag;  // 0-线程A工作 1-线程B工作 2-线程C工作
char* err;
void* threadA(void* arg)
{
	FILE* fp = fopen("ERROR.log", "r");
	if (fp == NULL)
	{
		perror("fopen failed");
		exit(0);
	}
	
	sleep(0);
	pthread_mutex_lock(&lock);
	while (err = fgets(buffer_A, sizeof(buffer_A), fp))
	{
		flag = 1;

		pthread_cond_signal(&cdB);
		while (flag == 1)
			pthread_cond_wait(&cdA, &lock);
	}
	flag = 1;
	pthread_cond_signal(&cdB);
	pthread_mutex_unlock(&lock);

	fclose(fp);
}

void* threadB(void* arg)
{
	pthread_mutex_lock(&lock);
	while (flag == 0 || flag == 2)
	{
		pthread_cond_wait(&cdB, &lock);
	}

	while (err)
	{
		if (strstr(buffer_A, KEY1) != NULL || strstr(buffer_A, KEY2) != NULL)
		{
			strcpy(buffer_B, buffer_A);
			flag = 2;
			pthread_cond_signal(&cdC);
			while (flag == 2)
			{
				pthread_cond_wait(&cdB, &lock);
			}
		}

		flag = 0;
		pthread_cond_signal(&cdA);
		while (flag == 0)
		{
			pthread_cond_wait(&cdB, &lock);
		}
	}
	flag = 2;
	pthread_cond_signal(&cdC);

	pthread_mutex_unlock(&lock);
}

void* threadC(void* arg)
{
	int fd = open("RESULT.log", O_WRONLY|O_CREAT|O_TRUNC, 0664);
	pthread_mutex_lock(&lock);
	while (flag == 0 || flag == 1)
	{
		pthread_cond_wait(&cdC, &lock);
	}

	while (err)
	{
		write(fd, buffer_B, strlen(buffer_B));
		flag = 1;
		pthread_cond_signal(&cdB);
		while (flag == 1)
		{
			pthread_cond_wait(&cdC, &lock);
		}
	}
	pthread_mutex_unlock(&lock);
	close(fd);
}

int main()
{
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cdA, NULL);
	pthread_cond_init(&cdB, NULL);
	pthread_cond_init(&cdC, NULL);

	pthread_t tid[3];
	pthread_create(&tid[0], NULL, threadA, NULL);
	pthread_create(&tid[1], NULL, threadB, NULL);
	pthread_create(&tid[2], NULL, threadC, NULL);

	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);
	pthread_join(tid[2], NULL);

	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cdA);
	pthread_cond_destroy(&cdB);
	pthread_cond_destroy(&cdC);
	
	return 0;
}
