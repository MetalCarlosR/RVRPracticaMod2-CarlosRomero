#include "Serializable.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

class Jugador : public Serializable
{
public:
    Jugador(const char *_n, int16_t _x, int16_t _y) : x(_x), y(_y)
    {
        strncpy(name, _n, MAX_NAME);
    };

    virtual ~Jugador(){};

    void to_bin()
    {
        int32_t data_size = MAX_NAME * sizeof(char) + 2 * sizeof(int16_t);
        alloc_data(data_size);

        char *tmp = _data;

        memcpy(tmp, name, MAX_NAME * sizeof(char));

        tmp += MAX_NAME * sizeof(char);

        memcpy(tmp, &x, sizeof(int16_t));

        tmp += MAX_NAME * sizeof(int16_t);

        memcpy(tmp, &y, sizeof(int16_t));
    }

    int from_bin(char *data)
    {
        char *tmp = data;

        memcpy(name, tmp, MAX_NAME * sizeof(char));

        tmp += MAX_NAME * sizeof(char);

        memcpy(&x, tmp, sizeof(int16_t));

        tmp += MAX_NAME * sizeof(int16_t);

        memcpy(&y, tmp, sizeof(int16_t));

        return 0;
    }

private:
    static const size_t MAX_NAME = 20;

    char name[MAX_NAME];

    int16_t x;
    int16_t y;
};

int main(int argc, char **argv)
{

    Jugador one_out("player one", 12, 345);

    int fd = open("./data_player", O_CREAT | O_TRUNC | O_RDWR, 0666);

    one_out.to_bin();

    write(fd, one_out.data(), one_out.size());

    return 0;
}