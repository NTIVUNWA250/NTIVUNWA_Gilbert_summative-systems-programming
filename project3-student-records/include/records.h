/*
 * records.h - student and course structures and the operations that create,
 * change and delete them. A StudentList owns a dynamic array of Student, and
 * each Student owns a dynamic array of Course.
 */
#ifndef RECORDS_H
#define RECORDS_H

#define NAME_LEN 32
#define PROGRAMME_LEN 48
#define CODE_LEN 12
#define COURSE_NAME_LEN 48
#define PASS_PERCENT 50.0

typedef enum {
    STATUS_OK = 0,
    STATUS_NO_MEMORY,
    STATUS_DUPLICATE,
    STATUS_NOT_FOUND,
    STATUS_INVALID,
    STATUS_FILE_ERROR
} Status;

typedef struct {
    char code[CODE_LEN];
    char name[COURSE_NAME_LEN];
    double marks;
    double max_marks;
} Course;

typedef struct {
    int id;
    char first_name[NAME_LEN];
    char last_name[NAME_LEN];
    char programme[PROGRAMME_LEN];
    int year;
    int course_count;
    Course *courses;
} Student;

typedef struct {
    Student *students;
    int count;
} StudentList;

const char *status_message(Status status);

void list_init(StudentList *list);
void list_free(StudentList *list);
Student *list_get(StudentList *list, int id);
Status list_add(StudentList *list, int id, const char *first_name,
                const char *last_name, const char *programme, int year);
Status list_remove(StudentList *list, int id);

Status student_update_info(Student *student, const char *first_name,
                           const char *last_name, const char *programme, int year);
Course *student_find_course(Student *student, const char *code);
Status student_add_course(Student *student, const char *code, const char *name,
                          double marks, double max_marks);
Status student_update_marks(Student *student, const char *code, double marks);

int student_average(const Student *student, double *average);
int course_percentage(const Course *course, double *percentage);
int course_passed(const Course *course);

#endif
