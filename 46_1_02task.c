#include <stdio.h>
#include <string.h>
#include "os_file.h"


typedef struct ListUnit
{
    char* unit_name;
    struct ListUnit* parent_unit;
    struct ListUnit* first_child_unit;
    struct ListUnit* next_unit;
    int file_size;
} ListUnit;


// -------------- GLOBAL VARIABLES ---------------
int GL_DISK_SIZE = 0, GL_FREE_MEMORY = 0;
struct ListUnit* GL_ROOT_DIR = NULL;
struct ListUnit* GL_CURRENT_DIR = NULL;


// --------------- LIST METHODS ----------------
int init_root()
{   
    ListUnit* unit_mem_buffer;
    char* name_mem_buffer;

    unit_mem_buffer = (ListUnit*)malloc(sizeof(ListUnit));
    if (unit_mem_buffer == NULL)
    {
        return 0;
    }
    GL_ROOT_DIR = unit_mem_buffer;
    
    name_mem_buffer = (char*)malloc(sizeof(char) * 2);
    if (name_mem_buffer == NULL)
    {   
        free(unit_mem_buffer);
        return 0;
    }
    GL_ROOT_DIR->unit_name = name_mem_buffer;

    strcpy(GL_ROOT_DIR->unit_name, "/\0");
    
    GL_ROOT_DIR->parent_unit = NULL;
    GL_ROOT_DIR->first_child_unit = NULL;
    GL_ROOT_DIR->next_unit = NULL;
    
    GL_ROOT_DIR->file_size = 0;
    
    return 1;
}

int add_unit(ListUnit* parent, const char* unit_name, int file_size)
{   
    if (parent->file_size > 0)
    {   
        return 0;
    }
    ListUnit* unit_mem_buffer;
    ListUnit* new_unit;
    unit_mem_buffer = (ListUnit*)malloc(sizeof(ListUnit));
    if (unit_mem_buffer == NULL)
    {
        return 0;
    }
    new_unit = unit_mem_buffer;

    int name_length = (int)strlen(unit_name) + 1;
    char* name_mem_buffer = (char*)malloc(sizeof(char) * name_length);
    if (name_mem_buffer == NULL)
    {
        free(new_unit);
        return 0;
    }
    new_unit->unit_name = name_mem_buffer;
    strcpy(new_unit->unit_name, unit_name);
    
    if (!(parent->first_child_unit))
    {   // если ветвь пустая, то добавляем unit как first_child_unit
        parent->first_child_unit = new_unit;
    }
    else
    {   
        ListUnit *buffer = parent->first_child_unit;
        while (buffer->next_unit)
        {
            buffer = buffer->next_unit;
        }
        // после цикла в buffer находится последний unit в данной ветви
        buffer->next_unit = new_unit;
    }
    
    GL_FREE_MEMORY -= file_size;

    new_unit->parent_unit = parent;

    new_unit->next_unit = NULL;
    new_unit->first_child_unit = NULL;

    new_unit->file_size = file_size;
    
    return 1;
}

ListUnit* find_unit(ListUnit* parent, const char* unit_name)
{   // parent - в каком юните начать поиски
    // в случае удачи возвращает указатель на unit, unit_name которого == unit_name
    if (strcmp(unit_name, "..") == 0)
    {   
        if (strcmp(GL_CURRENT_DIR->unit_name, "/") == 0)
        {
            return NULL;
        }
        return parent->parent_unit;
    }
    else if (strcmp(unit_name, ".") == 0)
    {
        return parent;
    }
    
    ListUnit* buffer = parent;
    if (buffer->first_child_unit)
    {
        buffer = buffer->first_child_unit;
        if (strcmp(buffer->unit_name, unit_name) == 0)  // если первый ребенок совпал по unit_name
        {   
            return buffer;
        }
        buffer = buffer->next_unit;
        while (buffer)
        {   
            if (strcmp(buffer->unit_name, unit_name) == 0)
            {   
                return buffer;
            }
            buffer = buffer->next_unit;
        }
        return NULL;
    }
    return NULL;
}

void delete_single_unit(ListUnit* unit_to_delete)
{
    // Случай 1: если удаляемый юнит является корнем 
    if (unit_to_delete->parent_unit == NULL)
    {
        free(unit_to_delete->unit_name);
        free(unit_to_delete);
        return;
    }
    // Случай 2: если удаляемый юнит является первым ребенком вершины
    if (strcmp(unit_to_delete->parent_unit->first_child_unit->unit_name, 
        unit_to_delete->unit_name) == 0)
    {        
        unit_to_delete->parent_unit->first_child_unit = unit_to_delete->next_unit;
    }
    else // Случай 3: если удаляемый юнит является промежуточной вершиной
    {   
        ListUnit* buffer = unit_to_delete->parent_unit->first_child_unit;
        while (strcmp(buffer->next_unit->unit_name, unit_to_delete->unit_name) != 0)
        {
            buffer = buffer->next_unit;
        }
        buffer->next_unit = unit_to_delete->next_unit;
    }
    
    if (unit_to_delete->file_size > 0) // если unit является файлом
    {
        GL_FREE_MEMORY += unit_to_delete->file_size;
    }

    free(unit_to_delete->unit_name);
    free(unit_to_delete);
    return;
}

void delete_unit_recursively(ListUnit* unit_to_delete)
{   
    if (unit_to_delete->first_child_unit == NULL)
    {   
        delete_single_unit(unit_to_delete);
        return;
    }
    ListUnit* buffer = unit_to_delete->first_child_unit;
    ListUnit* buffer_2;
    while (buffer)
    {   
        buffer_2 = buffer->next_unit;
        delete_unit_recursively(buffer);
        buffer = buffer_2;
    }
    delete_single_unit(unit_to_delete);
    return;
}


// ---------------- SERVICE FUNCTIONS --------------


void free_2d_arr(char** arr, int rows_quantity)
{   
    for (int i = 0; i < rows_quantity; i++)
    {
        free(arr[i]);
    }
    free(arr);
}

void sort_2d_arr(char*** arr, int rows_quantity, int begin_with)
{   
    char* buffer;
    for (int i = begin_with; i < rows_quantity; i++)
    {
        for (int j = begin_with; j < rows_quantity - 1; j++)
        {
            if (strcmp((*arr)[j], (*arr)[j + 1]) > 0)
            {
                buffer = (*arr)[j];
                (*arr)[j] = (*arr)[j + 1];
                (*arr)[j + 1] = buffer;
            }
        }
    }
}

char** parse_path(const char* path, int* unit_count, int is_absolute)
{   
    int path_length = strlen(path), letter_counter = 0, i = 0;
    char** unit_names = NULL;
    *unit_count = 0;

    while (i < path_length)
    {   
        if (path[i] == '/')
        {   
            if (i + 1 < path_length && path[i + 1] == '/')
            {
                free_2d_arr(unit_names, (*unit_count));
                return NULL;
            }
            i++;
            continue;  
        }

        unit_names = (char**)realloc(unit_names, sizeof(char*) * ((*unit_count) + 1));
        if (unit_names == NULL)
        {
            free_2d_arr(unit_names, (*unit_count));
            return NULL;
        }

        unit_names[(*unit_count)] = (char*)malloc(sizeof(char) * 33);
        if (unit_names[(*unit_count)] == NULL)
        {
            free_2d_arr(unit_names, (*unit_count));
            return NULL;
        }
        
        while (path[i] != '/')
        {   
            if (letter_counter == 32)
            {
                free_2d_arr(unit_names, (*unit_count) + 1);
                return NULL;
            }

            if ((path[i] >= ' ' && path[i] <= '-') || (path[i] >= ':' && path[i] <= '@')
                || (path[i] >= '\\' && path[i] <= '^') || path[i] == '`' || (path[i] >= '{' && path[i] <= '~'))
            {
                free_2d_arr(unit_names, (*unit_count) + 1);
                return NULL;
            }
            
            unit_names[(*unit_count)][letter_counter] = path[i];
            letter_counter++;
            i++;
            
            if (path[i] == '\0')
            {   
                break;
            }
        }
        
        if (letter_counter < 32)
        {   
            unit_names[(*unit_count)] = (char*)realloc(unit_names[(*unit_count)], sizeof(char) * (letter_counter + 1));
            if (unit_names[(*unit_count)] == NULL)
            {
                free_2d_arr(unit_names, (*unit_count) + 1);
                return NULL;
            }
        }

        unit_names[(*unit_count)][letter_counter] = '\0';
        
        letter_counter = 0;
        (*unit_count)++;
    }
    return unit_names;
}

int is_correct_name(char* unit_name)
{   
    if (unit_name[0] == '.')
    {
        return 0;
    }
    return 1;
}
// -------------------- FILE MANAGER FUNCTIONS --------------------

int my_create(int disk_size)
{   
    if (GL_DISK_SIZE != 0 || GL_ROOT_DIR != NULL || GL_CURRENT_DIR != NULL || disk_size <= 0)
    {
        return 0;
    }

    int err_code = init_root();
    if (err_code == 0)
    {
        return 0;
    }
    GL_CURRENT_DIR = GL_ROOT_DIR;

    GL_DISK_SIZE = disk_size;
    GL_FREE_MEMORY = disk_size;
    
    return 1;
}

int my_destroy()
{   
    if (GL_ROOT_DIR == NULL || GL_CURRENT_DIR == NULL)
    {
        return 0;
    }
    
    delete_unit_recursively(GL_ROOT_DIR);
    GL_ROOT_DIR = NULL;
    GL_CURRENT_DIR = NULL;
    GL_DISK_SIZE = 0;
    GL_FREE_MEMORY = 0;
    
    return 1;
}

int my_create_dir(const char* path)
{   
    if (GL_DISK_SIZE == 0)
    {
        return 0;
    }

    int is_absolute = 0;
    if (path[0] == '/')
    {
        if (strlen(path) == 1 || strlen(path) > 128)
        {
            return 0;
        }
        is_absolute = 1;
    }

    int unit_count = 0;
    char** unit_names = parse_path(path, &unit_count, is_absolute);
    if (unit_names == NULL || unit_count == 0)
    {
        return 0;
    }
    
    ListUnit* current_unit_buffer;
    if (is_absolute)
    {
        current_unit_buffer = GL_ROOT_DIR;
    }
    else
    {
        current_unit_buffer = GL_CURRENT_DIR;
    }
    
    for (int i = 0; i < unit_count; i++)
    {   
        ListUnit* buffer = find_unit(current_unit_buffer, unit_names[i]);
        if (buffer != NULL && i == (unit_count - 1))
        {// если уже создан и это последнее название, то ошибка уникальности
            free_2d_arr(unit_names, unit_count);
            return 0;
        }
        else if (buffer == NULL && i < (unit_count - 1))
        {// если не создан ПРОМЕЖУТОЧНЫЙ unit, то ошибка
            free_2d_arr(unit_names, unit_count);
            return 0;
        }
        else if (i == (unit_count - 1))
        {// если последнее имя в массиве, то создаем
            if (is_correct_name(unit_names[i]) == 0)
            {
                free_2d_arr(unit_names, unit_count);
                return 0;
            }
            int err_code = add_unit(current_unit_buffer, unit_names[i], 0);
            free_2d_arr(unit_names, unit_count);
            return err_code;
        }
        else
        {
            current_unit_buffer = buffer;
        }
    }
    free_2d_arr(unit_names, unit_count);
    return 0;
}

int my_create_file(const char* path, int file_size)
{   
    if (GL_DISK_SIZE == 0 || file_size <= 0 || file_size > GL_FREE_MEMORY)
    {
        return 0;
    }

    int is_absolute = 0;
    if (path[0] == '/')
    {
        if (strlen(path) == 1 || strlen(path) > 128)
        {
            return 0;
        }
        is_absolute = 1;
    }
    int unit_count = 0;
    char** unit_names = parse_path(path, &unit_count, is_absolute);
    if (unit_names == NULL || unit_count == 0)
    {
        return 0;
    }

    ListUnit* current_unit_buffer;
    if (is_absolute)
    {
        current_unit_buffer = GL_ROOT_DIR;
    }
    else
    {
        current_unit_buffer = GL_CURRENT_DIR;
    }

    for (int i = 0; i < unit_count; i++)
    {   
        ListUnit* buffer = find_unit(current_unit_buffer, unit_names[i]);
        if (buffer != NULL && i == (unit_count - 1))
        {// если уже создан и это последнее название, то ошибка уникальности
            free_2d_arr(unit_names, unit_count);
            return 0;
        }
        else if (buffer == NULL && i < (unit_count - 1))
        {// если не создан ПРОМЕЖУТОЧНЫЙ unit, то ошибка
            free_2d_arr(unit_names, unit_count);
            return 0;
        }
        else if (i == (unit_count - 1))
        {// если последнее имя в массиве, то создаем
            if (is_correct_name(unit_names[i]) == 0)
            {
                free_2d_arr(unit_names, unit_count);
                return 0;
            }
            int err_code = add_unit(current_unit_buffer, unit_names[i], file_size);
            free_2d_arr(unit_names, unit_count);
            return err_code;
        }
        else
        {   
            current_unit_buffer = buffer;
        }
    }
    free_2d_arr(unit_names, unit_count);
    return 0;
}

int my_remove(const char* path, int recursive)
{   
    if (GL_DISK_SIZE == 0 || recursive < 0 || recursive > 1)
    {
        return 0;
    }

    int is_absolute = 0;
    if (path[0] == '/')
    {
        if (strlen(path) == 1 || strlen(path) > 128)
        {
            return 0;
        }
        is_absolute = 1;
    }
    int unit_count = 0;
    char** unit_names = parse_path(path, &unit_count, is_absolute);
    if (unit_names == NULL || unit_count == 0)
    {
        return 0;
    }

    ListUnit* current_unit_buffer;
    if (is_absolute)
    {
        current_unit_buffer = GL_ROOT_DIR;
    }
    else
    {
        current_unit_buffer = GL_CURRENT_DIR;
    }
    
    for (int i = 0; i < unit_count; i++)
    {
        ListUnit* buffer = find_unit(current_unit_buffer, unit_names[i]); 
        if (buffer == NULL)
        {   
            free_2d_arr(unit_names, unit_count);
            return 0;
        }
        else if (i == unit_count - 1)
        {// непосредственно удаление unit
            free_2d_arr(unit_names, unit_count);
            if (buffer->first_child_unit && recursive != 1) 
            {   // если удаляемая папка непуста и флаг recursive не поднят
                return 0;
            }
            if (strcmp(GL_CURRENT_DIR->unit_name, buffer->unit_name) == 0)
            {
                GL_CURRENT_DIR = GL_ROOT_DIR;
            }
            delete_unit_recursively(buffer);
            return 1;
        }
        current_unit_buffer = buffer;
    }
    free_2d_arr(unit_names, unit_count);
    return 0;
}

int my_change_dir(const char* path)
{
    if (GL_DISK_SIZE == 0)
    {
        return 0;
    }

    int is_absolute = 0;
    if (path[0] == '/')
    {
        if (strlen(path) == 1 || strlen(path) > 128)
        {
            return 0;
        }
        is_absolute = 1;
    }
    int unit_count = 0;
    char** unit_names = parse_path(path, &unit_count, is_absolute);
    if (unit_names == NULL || unit_count == 0)
    {
        return 0;
    }

    ListUnit* current_unit_buffer;
    if (is_absolute)
    {
        current_unit_buffer = GL_ROOT_DIR;
    }
    else
    {
        current_unit_buffer = GL_CURRENT_DIR;
    }

    for (int i = 0; i < unit_count; i++)
    {
        ListUnit* buffer = find_unit(current_unit_buffer, unit_names[i]);
        current_unit_buffer = buffer;
        if (buffer == NULL)
        {   
            free_2d_arr(unit_names, unit_count);
            return 0;
        }
        else if (i == (unit_count - 1))
        {   
            if (current_unit_buffer->file_size > 0)
            {
                free_2d_arr(unit_names, unit_count);
                return 0;
            }
            GL_CURRENT_DIR = current_unit_buffer;
            free_2d_arr(unit_names, unit_count);
            return 1;
        }
    }
    free_2d_arr(unit_names, unit_count);
    return 0;
}

void my_get_cur_dir(char* dst) {
    if (GL_DISK_SIZE == 0 || dst == NULL) {
        return;
    }

    char* absolute_path = (char*)malloc(sizeof(char) * (strlen(GL_CURRENT_DIR->unit_name) + 1));
    if (absolute_path == NULL) {
        return;
    }
    strcpy(absolute_path, GL_CURRENT_DIR->unit_name);

    char* destination = NULL;

    ListUnit* buffer = GL_CURRENT_DIR->parent_unit;

    while (buffer) {
        size_t new_len = strlen(buffer->unit_name) + 2 + strlen(absolute_path);
        char* mem_buffer = (char*)realloc(destination, sizeof(char) * (new_len));
        if (mem_buffer == NULL) {
            free(absolute_path);
            free(destination);
            return;
        }
        destination = mem_buffer;

        strcpy(destination, buffer->unit_name);

        if (strcmp(buffer->unit_name, "/") != 0) {
            strcat(destination, "/");
        }

        strcat(destination, absolute_path);

        mem_buffer = (char*)realloc(absolute_path, sizeof(char) * (new_len));
        if (mem_buffer == NULL) {
            free(destination);
            return;
        }
        absolute_path = mem_buffer;

        strcpy(absolute_path, destination);

        buffer = buffer->parent_unit;
    }

    strcpy(dst, absolute_path);

    free(absolute_path);
    free(destination);
}

int my_list(const char* path, int dir_first)
{
    if (GL_DISK_SIZE == 0)
    {
        return 0;
    }

    int is_absolute = 0;
    if (path[0] == '/')
    {
        if (strlen(path) == 1 || strlen(path) > 128)
        {
            return 0;
        }
        is_absolute = 1;
    }
    int unit_count = 0;
    char** unit_names = parse_path(path, &unit_count, is_absolute);
    if (unit_names == NULL || unit_count == 0)
    {
        return 0;
    }

    ListUnit* current_unit_buffer;
    if (is_absolute)
    {
        current_unit_buffer = GL_ROOT_DIR;
    }
    else
    {
        current_unit_buffer = GL_CURRENT_DIR;
    }

    for (int i = 0; i < unit_count; i++)
    {
        ListUnit* buffer = find_unit(current_unit_buffer, unit_names[i]);
        
        if (buffer != NULL && i == (unit_count - 1))
        {// если создан и это последнее название, то начинаем выводить содержимое:
            free_2d_arr(unit_names, unit_count);
            if (buffer->file_size > 0)
            {
                printf("%s\n", path);
                return 1;
            }
            printf("%s:\n", path);
            buffer = buffer->first_child_unit;

            if (buffer == NULL)
            {   
                return 1;
            }

            ListUnit* buffer_2 = buffer;
            int file_count = 0, dir_count = 0;
            while (buffer)
            {
                if (buffer->file_size > 0)
                {
                    file_count++;
                }
                else if (buffer->file_size == 0)
                {
                    dir_count++;
                }
                buffer = buffer->next_unit;
            }

            int i = 0, child_count = dir_count + file_count;
            char** child_names = (char**)malloc(sizeof(char*) * child_count);
            if (child_names == NULL)
            {
                return 0;
            }

            if (dir_first)
            {   
                ListUnit* buffer_3 = buffer_2;
                while (buffer_2)
                {   
                    if (buffer_2->file_size == 0)
                    {
                        child_names[i] = (char*)malloc(sizeof(char) * (strlen(buffer_2->unit_name) + 1));
                        if (child_names[i] == NULL)
                        {   
                            free(child_names);
                            return 0;
                        }
                        strcpy(child_names[i], buffer_2->unit_name);
                        child_names[i][strlen(buffer_2->unit_name)] = '\0';
                        i++;
                    }
                    buffer_2 = buffer_2->next_unit;
                }
                sort_2d_arr(&child_names, dir_count, 0); // begin with 0
                for (int j = 0; j < dir_count; j++)
                {
                    printf("%s\n", child_names[j]);
                }
                
                while (buffer_3)
                {   
                    if (buffer_3->file_size > 0)
                    {
                        child_names[i] = (char*)malloc(sizeof(char) * (strlen(buffer_3->unit_name) + 1));
                        if (child_names[i] == NULL)
                        {
                            free(child_names);
                            return 0;
                        }
                        strcpy(child_names[i], buffer_3->unit_name);
                        child_names[i][strlen(buffer_3->unit_name)] = '\0';
                        i++;
                    }
                    buffer_3 = buffer_3->next_unit;
                }
                sort_2d_arr(&child_names, child_count, dir_count);
                for (int j = dir_count; j < child_count; j++)
                {
                    printf("%s\n", child_names[j]);
                }
                
            }
            else
            {// просто добавим в обычном порядке
                while (buffer_2)
                {
                    child_names[i] = (char*)malloc(sizeof(char) * (strlen(buffer_2->unit_name) + 1));
                    if (child_names[i] == NULL)
                    {
                        free(child_names);
                        return 0;
                    }
                    strcpy(child_names[i], buffer_2->unit_name);
                    child_names[i][strlen(buffer_2->unit_name)] = '\0';
                    i++;
                    buffer_2 = buffer_2->next_unit;
                }
                sort_2d_arr(&child_names, child_count, 0);
                for (int i = 0; i < child_count; i++)
                {
                    printf("%s\n", child_names[i]);
                }
            }
            free_2d_arr(child_names, child_count);
            return 1;
        }
        else if (buffer == NULL && i <= (unit_count - 1))
        {// если не создан какой-либо unit, то ошибка
            free_2d_arr(unit_names, unit_count);
            return 0;
        }
        else // движемся дальше по пути
        {
            current_unit_buffer = buffer;
        }
    }
    free_2d_arr(unit_names, unit_count);
    return 0;
}

void setup_file_manager(file_manager_t *fm) {
    fm->create = my_create;
    fm->destroy = my_destroy;
    fm->create_dir = my_create_dir;
    fm->create_file = my_create_file;
    fm->remove = my_remove;
    fm->change_dir = my_change_dir;
    fm->get_cur_dir = my_get_cur_dir;
    fm->list = my_list;
}
