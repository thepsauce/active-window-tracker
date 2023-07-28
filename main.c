#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

struct precise_time {
	time_t major;
	time_t minor;
};

struct record {
	char *name;
	struct precise_time *timings;
	size_t ntimings;
} *records;
size_t nrecords;

int
addrecord(const char *name, struct precise_time *timings, size_t ntimings)
{
	struct record *newrecords;

	fprintf(stderr, "log: adding record with name '%s' and '%zu' timings\n", name, ntimings);
	/* check if records already exists */
	for(size_t i = 0; i < nrecords; i++)
		if(!strcmp(records[i].name, name)) {
			/* concat timings */
			struct precise_time *newtimings;
			newtimings = realloc(records[i].timings, sizeof(*newtimings) * (records[i].ntimings + ntimings));
			if(!newtimings)
				return -1;
			memcpy(newtimings + records[i].ntimings, timings, ntimings);
			records[i].ntimings += ntimings;
			return 0;
		}
	/* add new record */
	newrecords = realloc(records, sizeof(*records) * (nrecords + 1));
	if(newrecords == NULL)
		return -1;
	records = newrecords;
	records[nrecords].name = strdup(name);
	if(records[nrecords].name == NULL)
		return -1;
	records[nrecords].timings = malloc(sizeof(*timings) * ntimings);
	if(records[nrecords].timings == NULL)
		return -1;
	memcpy(records[nrecords].timings, timings, sizeof(*timings) * ntimings);
	records[nrecords].ntimings = ntimings;
	return 0;
}

int
main(int argc, char **argv)
{
	FILE *fp;
	DIR *dir;
	struct dirent *ent;
	char *path, *newpath;
	const char *home;
	size_t n;
	const char *name;
	size_t nname;
	char *line = NULL;
	size_t nline = 0;
	size_t read;
	char *title = NULL, *newtitle;
	struct precise_time *timings, *newtimings;
	size_t ntimings;

	home = getenv("HOME");
	if(home == NULL)
		goto memerr;
	n = strlen(home);
	path = malloc(n + 30);
	if(path == NULL)
		goto memerr;
	strcpy(path, home);
	strcat(path, "/.tracks/");
	n += sizeof("/.tracks/") - 1;

	timings = malloc(sizeof(*timings) * 20);
	ntimings = 0;
	if(timings == NULL)
		goto memerr;

	dir = opendir(path);
	if(!dir) {
		fprintf(stderr, "could not open directory '%s'\n", path);
		return -1;
	}
	while((ent = readdir(dir)) != NULL) {
		name = ent->d_name;
		if(name[0] == '.')
			continue;
		nname = strlen(name);
		newpath = realloc(path, n + nname + 1);
		if(newpath == NULL)
			goto memerr;
		path = newpath;
		strcpy(path + n, name);
		fp = fopen(path, "r");
		if(fp == NULL)
			fprintf(stderr,
				"warn: could not open file '%s'\n",
				path);
		fprintf(stderr, "log: opened file '%s'\n", path);
		while((read = getline(&line, &nline, fp)) != -1) {
			char *ptr;

			newtitle = realloc(title, read);
			if(newtitle == NULL)
				goto memerr;
			title = newtitle;
			memcpy(title, line, read);
			title[read - 1] = 0;
			if(title == NULL)
				goto memerr;
			if(getline(&line, &nline, fp) == -1)
				goto corrupterr;
			ntimings = 0;
			ptr = line;
			while(1) {
				time_t major, minor;

				if(!isdigit(*ptr))
					goto corrupterr;
				major = strtoull(ptr, &ptr, 10);
				if(*ptr != '.')
					goto corrupterr;
				ptr++;
				if(!isdigit(*ptr))
					goto corrupterr;
				minor = strtoull(ptr, &ptr, 10);
				newtimings = realloc(timings,
					sizeof(*timings) * (ntimings + 1));
				if(newtimings == NULL)
					goto memerr;
				timings = newtimings;
				timings[ntimings].major = major;
				timings[ntimings].minor = minor;
				ntimings++;
				if(*ptr == '\n')
					break;
				ptr++;
			}
			if((ntimings % 2) == 1)
				goto corrupterr;
			if(addrecord(title, timings, ntimings) < 0)
				goto memerr;
			continue;
		corrupterr:
			fprintf(stderr,
				"warn: corrupt file '%s'\n",
				path);
			break;
		}
		fclose(fp);
	}
	free(line);
	free(title);
	free(timings);
	closedir(dir);
	free(path);
	return 0;
memerr:
	fprintf(stderr, "fatal: out of memory\n");
	return -1;
}
