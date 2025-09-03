#define STRINGSIZE_DETAIL(x) #x
#define STRINGSIZE(x) STRINGSIZE_DETAIL(x)
#define logdetail(msg) (msg ", " __FILE__ ":" STRINGSIZE(__LINE__))

#define printer_func (__func__ ":" STRINGSIZE(__FILE__))
