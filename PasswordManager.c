#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAXDATA 512
#define MAXROWS 100


struct Login {
    int id;
    char set;
    char name[MAXDATA];
    char username[MAXDATA];
    char password[128];
};

struct Database {
    struct Login log[MAXDATA];
};

struct Connection {
    FILE *file;
    struct Database *db;
};

void die(char *message) {
    if(errno) {
        perror(message);
    } else {
        printf("Error: %s\n", message);
    }
    exit(1);
}

struct Connection *Database_open(const char *filename, char action) {
    struct Connection *conn = malloc(sizeof(struct Connection)); if(!conn) die("Memory problem.");

    conn->db = malloc(sizeof(struct Database)); if(!conn->db) die("Memory problem.");

    if (action == 'c') {
        conn->file = fopen(filename, "w");
    } else {
        conn->file = fopen(filename, "r+");

        if (conn->file) fread(conn->db, sizeof(struct Database), 1 , conn->file);
    } if (!conn->file) die("Failed to open the file.");
    return conn;
}

void Database_write(struct Connection *conn) {
    rewind(conn->file);

    int res = fwrite(conn->db, sizeof(struct Database), 1, conn->file); if (res != 1) die("Failed to Write the Database into the file.");

    res = fflush(conn->file); if (res == -1) die("Failed to flush the Database.");
}

void Database_close(struct Connection *conn) {
    if (conn) {
        fclose(conn->file);
        free(conn->db);
        free(conn);
    }
}

void Database_create(struct Connection *conn) {
    for (int i = 0; i < MAXROWS; i++) {
        struct Login log = { .id = i, .set = 0 };
        conn->db->log[i] = log;
    }
}

void Database_set(struct Connection *conn, int id, char *name, char *username, char *password) {
    struct Login *log = &conn->db->log[id];

    log->set = 1;

    char *res = strncpy(log->name, name, MAXDATA);
    if (!res) die("Failed to copy name.");

    res = strncpy(log->username, username, MAXDATA);
    if (!res) die("Failed to copy username.");

    strncpy(log->password, password, 128);
    if (!res) die("Failed to copy password.");
}

void Database_get(struct Connection *conn, int id) {
    struct Login *log = &conn->db->log[id];

    if (conn->db->log[id].set == 0) {
        die("Can't get (Not set yet).");
    }
    printf("%d\t%s\t%s\t%s\n", id, log->name, log->username, log->password);
}

void Database_delete(struct Connection *conn, int id) {
    struct Login log = { .id = id, .set = 0};
    if (conn->db->log[id].set == 0) {
        die("Can't delete (Not set yet).");
    }
    conn->db->log[id] = log;
}


int main(int argc, char *argv[]) {

    if (argc < 3) die("Invalid syntax\nUsage: <Filename> <Action> [Parameters]");

    char action = argv[2][0];
    char *filename = argv[1];

    int id = 0;

    if (argc > 3) id = atoi(argv[3]);
    
    struct Connection *conn = Database_open(filename, action); 

    switch (action) {
        case 'c':
            if (argc != 3) die("Invalid syntax\nUsage: <Filename> c") ;
            Database_create(conn);
            Database_write(conn);
            break;

        case 's':
            if (argc != 7) die("Invalid syntax\nUsage: <Filename> s <id> <name> <username> <password>");
            Database_set(conn, id, argv[4], argv[5], argv[6]);
            Database_write(conn);
            break;

        case 'g':
            if (argc != 4) die("Invalid syntax\nUsage: <Filename> g <id>");
            Database_get(conn, id);
            break;

        case 'd':
            if (argc != 4) die("Invalid syntax\nUsage: <Filename> d <id>");
            Database_delete(conn, id);
            Database_write(conn);   
            break;

        case 'l':
            if (argc != 3) die("Invalid syntax\nUsage: <Filename> l");
            for (int i = 0; i < MAXROWS; i++) {
                Database_get(conn, i);
            }
            break;
    }
    Database_close(conn);
    return 0;
}