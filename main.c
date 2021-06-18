#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// for i2c in linux, refer to: https://www.kernel.org/doc/html/latest/i2c/index.html


// for i2c_master_get and i2c_master_set, refer to: https://www.kernel.org/doc/Documentation/i2c/smbus-protocol


// for I2C ioctl request codes, refer to: https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/i2c-dev.h

#define I2C_SLAVE 1795


int i2c_master_open(const char * i2c_device, unsigned char i2c_slave_address)
{
    int i2c_fd = open(i2c_device, O_RDWR | O_NONBLOCK);
    if(i2c_fd == -1)
    {
        return -1;
    }
    int res = ioctl(i2c_fd, I2C_SLAVE, i2c_slave_address);
    if(res == -1)
    {
        close(i2c_fd);
        return -1;
    }
    return i2c_fd;
}

ssize_t i2c_master_read(int i2c_fd, unsigned char * buf, size_t count)
{
    return read(i2c_fd, buf, count);
}

ssize_t i2c_master_write(int i2c_fd, unsigned char * buf, size_t count)
{
    return write(i2c_fd, buf, count);
}

int i2c_master_close(int i2c_fd)
{
    return close(i2c_fd);
}

// return 1 byte for the given command (if -1, then error)
ssize_t i2c_master_get(int i2c_fd, unsigned char slave_command)
{
    if(i2c_master_write(i2c_fd, &slave_command, 1) == -1)
    {
        return -1;
    }
    unsigned char buf[1];
    if(i2c_master_read(i2c_fd, buf, 1) == -1)
    {
        return -1;
    }
    return buf[0];
}

// set 1 byte value for the given command (if -1, then error)
ssize_t i2c_master_set(int i2c_fd, unsigned char slave_command, unsigned char value)
{
    unsigned char buf[2];
    buf[0] = slave_command;
    buf[1] = value;
    return i2c_master_write(i2c_fd, buf, 2);
}

void print_help()
{
    // 80c: ---------------------------------------------------------------------------------
    printf("Usage: i2c-ctl [options] [device] {get|set} <args...> ...\n"
           "This tool can send or receive a register byte in a slave I2C device (SMBus).\n"
           "\n"
           "Options:\n"
           "  --device,-d                    I2C device path (defaults to: \"/dev/i2c\").\n"
           "                                 This the default option, and may also be given\n"
           "                                 directly without any option flag.\n"
           "\n"
           "  --slave-address,--address,-a   I2C slave address.\n"
           "\n"
           "  @<address>                     I2C slave address, as given in <address>.\n"
           "\n"
           "  --format,-f                    Print get-result with given printf-format.\n"
           "                                 Defaults to \"0x\%02x\\n\".\n"
           "\n"
           "  --verbose,-v                   Print info messages to stderr.\n"
           "\n"
           "  --help,-h                      Show this help.\n"
           "\n"
           "\n"
           "Actions:\n"
           "  get [register]\n"
           "  \n"
           "  set [register] [value]\n"
           "\n"
           "\n"
           "Examples:\n"
           "  > i2c-ctl /dev/i2c-1 @0x68 set 0x44 0x3a\n"
           "  > i2c-ctl /dev/i2c-1 @0x68 get 0x44\n"
           "  0x3a\n"
           "  > i2c-ctl /dev/i2c-1 @104 get 0x44\n"
           "  0x3a\n"
           "  > i2c-ctl -d /dev/i2c-1 -a $'\\x68' get 0x44\n"
           "  0x3a\n"
           "  > i2c-ctl -f $'\%d\\n' /dev/i2c-1 @0x68 get 0x44\n"
           "  58\n"
           "\n"
           "\n"
           "Note: Values or addresses can be passed as hexadecimal (0x##), integer (#), or"
           "directly as a raw char (may not be printable).\n"
           "\n");
}

unsigned char parse_byte(const char * str)
{
    if(str[0] == '0' && str[1] == 'x')
    {
        // hexadecimal representation (i.e. 0x00)
        return (int) strtol(str, NULL, 0);
    }
    else if((str[0] >= '0' && str[0] <= '9') || (str[0] == '-' && str[1] >= '0' && str[1] <= '9'))
    {
        // integer representation
        return (int) strtol(str, NULL, 0);
    }
    else
    {
        // raw byte (\0 if empty string)
        return str[0];
    }
}

int main(int argc, char * argv[])
{
    int verbose = 0;
    const char * i2c_device = "/dev/i2c";
    unsigned char i2c_slave_address = 0;
    const char * result_format = "0x%02x\n";
    
    for(int i=1;i<argc;++i)
    {
        char * arg = argv[i];
        
        if(strcmp(arg, "--") == 0)
        {
            if(i + 1 < argc)
            {
                i2c_device = argv[++i];
            }
        }
        else if(strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0)
        {
            verbose = 1;
        }
        else if(strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            print_help();
            return 0;
        }
        else if(strcmp(arg, "-f") == 0 || strcmp(arg, "--format") == 0)
        {
            if(i + 1 < argc)
            {
                result_format = argv[++i];
            }
            else
            {
                printf("error: Invalid usage. Expected a value for argument (%s).\n", arg);
                return 1;
            }
        }
        else if(strcmp(arg, "-d") == 0 || strcmp(arg, "--device") == 0)
        {
            if(i + 1 < argc)
            {
                i2c_device = argv[++i];
            }
            else
            {
                printf("error: Invalid usage. Expected a value for argument (%s).\n", arg);
                return 1;
            }
        }
        else if(arg[0] == '@' || strcmp(arg, "-a") == 0 || strcmp(arg, "--slave-address") == 0 || strcmp(arg, "--address") == 0)
        {
            char * arg_value;
            if(arg[0] == '@')
            {
                arg_value = arg + 1; // + 1 => skip first character (@)
            }
            else if(i + 1 < argc)
            {
                arg_value = argv[++i];
            }
            else
            {
                printf("error: Invalid usage. Expected a value for argument (%s).\n", arg);
                return 1;
            }
            
            i2c_slave_address = parse_byte(arg_value);
        }
        else if(strcmp(arg, "get") == 0 || strcmp(arg, "set") == 0)
        {
            // commands are handled later on, after a command is given, no options may be provided anymore
            break;
        }
        else
        {
            if(access(arg, F_OK) != -1)
            {
                i2c_device = arg;
            }
            else
            {
                printf("warning: Did not parse argument as device, file does not exist (%s).\n", arg);
            }
        }
    }
    
    if(verbose == 1) printf("info: I2C device = %s\n", i2c_device);
    if(verbose == 1) printf("info: I2C slave address = %d\n", i2c_slave_address);
    
    // open connection to local I2C device
    int fd = i2c_master_open(i2c_device, i2c_slave_address);
    if(fd == -1)
    {
        printf("error: Could not open I2C device (%s) for slave address (0x%x).\n", i2c_device, i2c_slave_address);
        return 1;
    }
    
    if(verbose == 1) printf("info: I2C device opened.\n");
    
    // execute commands
    for(int i=1;i<argc;++i)
    {
        char * arg = argv[i];
        
        if(strcmp(arg, "get") == 0)
        {
            if(i + 1 >= argc)
            {
                printf("error: Invalid usage. Expected a value for argument (%s).\n", arg);
                if(i2c_master_close(fd) != -1)
                {
                    if(verbose == 1) printf("info: I2C device closed.\n");
                }
                return 1;
            }
            
            char * arg_value = argv[++i];
            
            unsigned char cmd = parse_byte(arg_value);
            
            if(verbose == 1) printf("info: I2C get: 0x%x\n", cmd);
            
            ssize_t res = i2c_master_get(fd, cmd);
            
            if(res == -1)
            {
                printf("error: Could not get value for command (0x%x) for slave address (0x%x) using I2C device (%s).\n", cmd, i2c_slave_address, i2c_device);
                if(i2c_master_close(fd) != -1)
                {
                    if(verbose == 1) printf("info: I2C device closed.\n");
                }
                return 1;
            }
            
            printf(result_format, res);
        }
        else if(strcmp(arg, "set") == 0)
        {
            if(i + 2 >= argc)
            {
                printf("error: Invalid usage. Expected two values for argument (%s).\n", arg);
                if(i2c_master_close(fd) != -1)
                {
                    if(verbose == 1) printf("info: I2C device closed.\n");
                }
                return 1;
            }
            
            char * arg_value_1 = argv[++i];
            char * arg_value_2 = argv[++i];
            
            unsigned char cmd = parse_byte(arg_value_1);
            unsigned char value = parse_byte(arg_value_2);
            
            if(verbose == 1) printf("info: I2C set: 0x%x = 0x%x\n", cmd, value);
            
            ssize_t res = i2c_master_set(fd, cmd, value);
            
            if(res == -1)
            {
                printf("error: Could not set value (0x%x) for command (0x%x) for slave address (0x%x) using I2C device (%s).\n", cmd, value, i2c_slave_address, i2c_device);
                if(i2c_master_close(fd) != -1)
                {
                    if(verbose == 1) printf("info: I2C device closed.\n");
                }
                return 1;
            }
            
            if(verbose == 1) printf("info: I2C set success (%d).\n", res);
        }
    }
    
    
    if(i2c_master_close(fd) != -1)
    {
        if(verbose == 1) printf("info: I2C device closed.\n");
    }
    
    return 0;
}
