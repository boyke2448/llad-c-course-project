#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"


int output_file(int fd, struct dbheader_t *header, struct employee_t *employees) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    header->magic = htonl(header->magic);
	header->version = htons(header->version);
	header->count = htons(header->count);
	header->filesize = htonl(header->filesize);

	lseek(fd, 0, SEEK_SET);

	write(fd, header, sizeof(struct dbheader_t));
	printf("Wrote header to file\n");
	printf("Magic: 0x%x\n", ntohl(header->magic));
	printf("Version: %u\n", ntohs(header->version));
	printf("Count: %u\n", ntohs(header->count));
	printf("Filesize: %u\n", ntohl(header->filesize));

	for(int i = 0; i < ntohs(header->count); i++) {
		employees[i].hours = htonl(employees[i].hours);
	}
	write(fd, employees, ntohs(header->count) * sizeof(struct employee_t));

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
	if (fd < 0) {
		printf("Got a bad FD from the user\n");
		return STATUS_ERROR;
	}

	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Malloc failed create a db header\n");
		return STATUS_ERROR;
	}

	if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
		perror("read");
		free(header);
		return STATUS_ERROR;
	}

	header->version = ntohs(header->version);
	header->count = ntohs(header->count);
	header->magic = ntohl(header->magic);
	header->filesize = ntohl(header->filesize);

	printf("Read header from file\n");
	printf("Magic: 0x%x\n", header->magic);
	printf("Version: %u\n", header->version);
	printf("Count: %u\n", header->count);
	printf("Filesize: %u\n", header->filesize);

	if (header->magic != HEADER_MAGIC) {
		printf("Impromper header magic\n");
		free(header);
		return STATUS_ERROR;
	}


	if (header->version != 1) {
		printf("Impromper header version\n");
		free(header);
		return STATUS_ERROR;
	}

	struct stat dbstat = {0};
	fstat(fd, &dbstat);
	if (header->filesize != dbstat.st_size) {
		printf("Corrupted database\n");
		printf("Expected filesize: %u, actual filesize: %ld\n", header->filesize, dbstat.st_size);
		free(header);
		return STATUS_ERROR;
	}

	*headerOut = header;

	return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) {
	if (headerOut == NULL) {
		printf("Got a null pointer from the user\n");
		return STATUS_ERROR;
	}

	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Malloc failed to create db header\n");
		return STATUS_ERROR;
	}

	header->version = 0x1;
	header->count = 0;
	header->magic = HEADER_MAGIC;
	header->filesize = sizeof(struct dbheader_t);

	*headerOut = header;

	return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *header, struct employee_t **employeesOut) {
	if (fd < 0) {
		printf("Got a bad FD from the user\n");
		return STATUS_ERROR;
	}

	if (header == NULL) {
		printf("Got a null header from the user\n");
		return STATUS_ERROR;
	}

	struct employee_t *employees = calloc(header->count, sizeof(struct employee_t));
	if (employees == NULL && header->count != 0) {
		printf("Malloc failed to create employees array\n");
		return STATUS_ERROR;
	}

	if (read(fd, employees, header->count * sizeof(struct employee_t)) != header->count * sizeof(struct employee_t)) {
		perror("read");
		free(employees);
		return STATUS_ERROR;
	}

	for (int i = 0; i < header->count; i++) {
		employees[i].hours = ntohl(employees[i].hours);
	}

	*employeesOut = employees;

	return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *header, struct employee_t **employees, char *addstring) {
	if(header == NULL || employees == NULL || addstring == NULL) {
		printf("Got a null pointer from the user\n");
		return STATUS_ERROR;
	}

	struct employee_t new_employee = {0};

	char *token = strtok(addstring, ",");
	if(token == NULL) {
		printf("Invalid add string format\n");
		return STATUS_ERROR;
	}
	strncpy(new_employee.name, token, sizeof(new_employee.name) - 1);

	token = strtok(NULL, ",");
	if(token == NULL) {
		printf("Invalid add string format\n");
		return STATUS_ERROR;
	}
	strncpy(new_employee.address, token, sizeof(new_employee.address) - 1);

	token = strtok(NULL, ",");
	if(token == NULL) {
		printf("Invalid add string format\n");
		return STATUS_ERROR;
	}
	new_employee.hours = (unsigned int)atoi(token);

	struct employee_t *updated_employees = realloc(*employees, (header->count + 1) * sizeof(struct employee_t));
	if(updated_employees == NULL) {
		printf("Realloc failed to expand employees array\n");
		return STATUS_ERROR;
	}

	updated_employees[header->count] = new_employee;
	*employees = updated_employees;

	header->count += 1;
	header->filesize += sizeof(struct employee_t);
	printf("Total sizeof employees: %zu\n", sizeof(*updated_employees));
	return STATUS_SUCCESS;
}
