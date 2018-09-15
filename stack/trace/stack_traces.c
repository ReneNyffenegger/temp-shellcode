//
//    https://gist.github.com/jvranish/4441299
//

/* compile with:
on linux:   gcc -g stack_traces.c
on OS X:    gcc -g -fno-pie stack_traces.c
on windows: gcc -g stack_traces.c -limagehlp
*/

#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#ifdef _WIN32
  #include <windows.h>
  #include <imagehlp.h>
#else
  #include <err.h>
  #include <execinfo.h>
#endif

#ifdef AS_DLL
#define func_declspec     __declspec(dllexport)
#define call_convention   __stdcall
#else
#define func_declspec
#define call_convention
#endif

// void almost_c99_signal_handler(int sig)
// {
//   switch(sig)
//   {
//     case SIGABRT:
//       fputs("Caught SIGABRT: usually caused by an abort() or assert()\n", stderr);
//       break;
//     case SIGFPE:
//       fputs("Caught SIGFPE: arithmetic exception, such as divide by zero\n", stderr);
//       break;
//     case SIGILL:
//       fputs("Caught SIGILL: illegal instruction\n", stderr);
//       break;
//     case SIGINT:
//       fputs("Caught SIGINT: interactive attention signal, probably a ctrl+c\n", stderr);
//       break;
//     case SIGSEGV:
//       fputs("Caught SIGSEGV: segfault\n", stderr);
//       break;
//     case SIGTERM:
//     default:
//       fputs("Caught SIGTERM: a termination request was sent to the program\n", stderr);
//       break;
//   }
//   _Exit(1);
// }

// void set_signal_handler()
// {
//   signal(SIGABRT, almost_c99_signal_handler);
//   signal(SIGFPE,  almost_c99_signal_handler);
//   signal(SIGILL,  almost_c99_signal_handler);
//   signal(SIGINT,  almost_c99_signal_handler);
//   signal(SIGSEGV, almost_c99_signal_handler);
//   signal(SIGTERM, almost_c99_signal_handler);
// }


static char const * icky_global_program_name;

/* Resolve symbol name and source location given the path to the executable 
   and an address */
int addr2line(char const * const program_name, void const * const addr)
{
  char addr2line_cmd[512] = {0};

  /* have addr2line map the address to the relent line in the code */
  #ifdef __APPLE__
    /* apple does things differently... */
    sprintf(addr2line_cmd,"atos -o %.256s %p", program_name, addr); 
  #else
    sprintf(addr2line_cmd,"addr2line -f -p -e %.256s %p", program_name, addr); 
  #endif

  return system(addr2line_cmd);
}


#ifdef _WIN32

#ifdef AS_DLL 
  #define notify_error(e) MessageBox(0, #e, 0, 0);
#else
  #define notify_error(e) fputs("Error: " #e "\n", stderr);
#endif

  void windows_print_stacktrace(CONTEXT* context)
  {
    SymInitialize(GetCurrentProcess(), 0, true);

    STACKFRAME frame = { 0 };

    /* setup initial stack frame */
    frame.AddrPC.Offset         = context->Eip;
    frame.AddrPC.Mode           = AddrModeFlat;
    frame.AddrStack.Offset      = context->Esp;
    frame.AddrStack.Mode        = AddrModeFlat;
    frame.AddrFrame.Offset      = context->Ebp;
    frame.AddrFrame.Mode        = AddrModeFlat;

    while (StackWalk(IMAGE_FILE_MACHINE_I386 ,
                     GetCurrentProcess(),
                     GetCurrentThread(),
                     &frame,
                     context,
                     0,
                     SymFunctionTableAccess,
                     SymGetModuleBase,
                     0 ) )
    {
      addr2line(icky_global_program_name, (void*)frame.AddrPC.Offset);
    }

    SymCleanup( GetCurrentProcess() );
  }

  LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
  {
    MessageBox(0, "in windows_exception_handler", 0, 0);
    switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
    {
      case EXCEPTION_ACCESS_VIOLATION:
        notify_error("Error: EXCEPTION_ACCESS_VIOLATION");
        break;
      case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        notify_error("Error: EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
        break;
      case EXCEPTION_BREAKPOINT:
        notify_error("Error: EXCEPTION_BREAKPOINT");
        break;
      case EXCEPTION_DATATYPE_MISALIGNMENT:
        notify_error("Error: EXCEPTION_DATATYPE_MISALIGNMENT");
        break;
      case EXCEPTION_FLT_DENORMAL_OPERAND:
        notify_error("Error: EXCEPTION_FLT_DENORMAL_OPERAND");
        break;
      case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        notify_error("Error: EXCEPTION_FLT_DIVIDE_BY_ZERO");
        break;
      case EXCEPTION_FLT_INEXACT_RESULT:
        notify_error("Error: EXCEPTION_FLT_INEXACT_RESULT");
        break;
      case EXCEPTION_FLT_INVALID_OPERATION:
        notify_error("Error: EXCEPTION_FLT_INVALID_OPERATION");
        break;
      case EXCEPTION_FLT_OVERFLOW:
        notify_error("Error: EXCEPTION_FLT_OVERFLOW");
        break;
      case EXCEPTION_FLT_STACK_CHECK:
        notify_error("Error: EXCEPTION_FLT_STACK_CHECK");
        break;
      case EXCEPTION_FLT_UNDERFLOW:
        notify_error("Error: EXCEPTION_FLT_UNDERFLOW");
        break;
      case EXCEPTION_ILLEGAL_INSTRUCTION:
        notify_error("Error: EXCEPTION_ILLEGAL_INSTRUCTION");
        break;
      case EXCEPTION_IN_PAGE_ERROR:
        notify_error("Error: EXCEPTION_IN_PAGE_ERROR");
        break;
      case EXCEPTION_INT_DIVIDE_BY_ZERO:
        notify_error("Error: EXCEPTION_INT_DIVIDE_BY_ZERO");
        break;
      case EXCEPTION_INT_OVERFLOW:
        notify_error("Error: EXCEPTION_INT_OVERFLOW");
        break;
      case EXCEPTION_INVALID_DISPOSITION:
        notify_error("Error: EXCEPTION_INVALID_DISPOSITION");
        break;
      case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        notify_error("Error: EXCEPTION_NONCONTINUABLE_EXCEPTION");
        break;
      case EXCEPTION_PRIV_INSTRUCTION:
        notify_error("Error: EXCEPTION_PRIV_INSTRUCTION");
        break;
      case EXCEPTION_SINGLE_STEP:
        notify_error("Error: EXCEPTION_SINGLE_STEP");
        break;
      case EXCEPTION_STACK_OVERFLOW:
        notify_error("Error: EXCEPTION_STACK_OVERFLOW");
        break;
      default:
        notify_error("Error: Unrecognized Exception");
        break;
    }
    fflush(stderr);
    /* If this is a stack overflow then we can't walk the stack, so just show
      where the error happened */
    if (EXCEPTION_STACK_OVERFLOW != ExceptionInfo->ExceptionRecord->ExceptionCode)
    {
        windows_print_stacktrace(ExceptionInfo->ContextRecord);
    }
    else
    {
        addr2line(icky_global_program_name, (void*)ExceptionInfo->ContextRecord->Eip);
    }

    return EXCEPTION_EXECUTE_HANDLER;
  }

  void set_signal_handler()
  {
    MessageBox(0, "Going to set_signal_handler", 0, 0);
    SetUnhandledExceptionFilter(windows_exception_handler);
  }
#else

  #define MAX_STACK_FRAMES 64
  static void *stack_traces[MAX_STACK_FRAMES];
  void posix_print_stack_trace()
  {
    int i, trace_size = 0;
    char **messages = (char **)NULL;

    trace_size = backtrace(stack_traces, MAX_STACK_FRAMES);
    messages = backtrace_symbols(stack_traces, trace_size);

    /* skip the first couple stack frames (as they are this function and
     our handler) and also skip the last frame as it's (always?) junk. */
    // for (i = 3; i < (trace_size - 1); ++i)
    for (i = 0; i &lt; trace_size; ++i) // we'll use this for now so you can see what's going on
    {
      if (addr2line(icky_global_program_name, stack_traces[i]) != 0)
      {
        printf("  error determining line # for: %s\n", messages[i]);
      }

    }
    if (messages) { free(messages); } 
  }

  void posix_signal_handler(int sig, siginfo_t *siginfo, void *context)
  {
    (void)context;
    switch(sig)
    {
      case SIGSEGV:
        fputs("Caught SIGSEGV: Segmentation Fault\n", stderr);
        break;
      case SIGINT:
        fputs("Caught SIGINT: Interactive attention signal, (usually ctrl+c)\n", stderr);
        break;
      case SIGFPE:
        switch(siginfo->si_code)
        {
          case FPE_INTDIV:
            fputs("Caught SIGFPE: (integer divide by zero)\n", stderr);
            break;
          case FPE_INTOVF:
            fputs("Caught SIGFPE: (integer overflow)\n", stderr);
            break;
          case FPE_FLTDIV:
            fputs("Caught SIGFPE: (floating-point divide by zero)\n", stderr);
            break;
          case FPE_FLTOVF:
            fputs("Caught SIGFPE: (floating-point overflow)\n", stderr);
            break;
          case FPE_FLTUND:
            fputs("Caught SIGFPE: (floating-point underflow)\n", stderr);
            break;
          case FPE_FLTRES:
            fputs("Caught SIGFPE: (floating-point inexact result)\n", stderr);
            break;
          case FPE_FLTINV:
            fputs("Caught SIGFPE: (floating-point invalid operation)\n", stderr);
            break;
          case FPE_FLTSUB:
            fputs("Caught SIGFPE: (subscript out of range)\n", stderr);
            break;
          default:
            fputs("Caught SIGFPE: Arithmetic Exception\n", stderr);
            break;
        }
      case SIGILL:
        switch(siginfo->si_code)
        {
          case ILL_ILLOPC:
            fputs("Caught SIGILL: (illegal opcode)\n", stderr);
            break;
          case ILL_ILLOPN:
            fputs("Caught SIGILL: (illegal operand)\n", stderr);
            break;
          case ILL_ILLADR:
            fputs("Caught SIGILL: (illegal addressing mode)\n", stderr);
            break;
          case ILL_ILLTRP:
            fputs("Caught SIGILL: (illegal trap)\n", stderr);
            break;
          case ILL_PRVOPC:
            fputs("Caught SIGILL: (privileged opcode)\n", stderr);
            break;
          case ILL_PRVREG:
            fputs("Caught SIGILL: (privileged register)\n", stderr);
            break;
          case ILL_COPROC:
            fputs("Caught SIGILL: (coprocessor error)\n", stderr);
            break;
          case ILL_BADSTK:
            fputs("Caught SIGILL: (internal stack error)\n", stderr);
            break;
          default:
            fputs("Caught SIGILL: Illegal Instruction\n", stderr);
            break;
        }
        break;
      case SIGTERM:
        fputs("Caught SIGTERM: a termination request was sent to the program\n", stderr);
        break;
      case SIGABRT:
        fputs("Caught SIGABRT: usually caused by an abort() or assert()\n", stderr);
        break;
      default:
        break;
    }
    posix_print_stack_trace();
    _Exit(1);
  }

  static uint8_t alternate_stack[SIGSTKSZ];
  void set_signal_handler()
  {
    /* setup alternate stack */
    {
      stack_t ss = {};
      /* malloc is usually used here, I'm not 100% sure my static allocation
         is valid but it seems to work just fine. */
      ss.ss_sp = (void*)alternate_stack;
      ss.ss_size = SIGSTKSZ;
      ss.ss_flags = 0;

      if (sigaltstack(&ss, NULL) != 0) { err(1, "sigaltstack"); }
    }

    /* register our signal handlers */
    {
      struct sigaction sig_action = {};
      sig_action.sa_sigaction = posix_signal_handler;
      sigemptyset(&sig_action.sa_mask);

      #ifdef __APPLE__
          /* for some reason we backtrace() doesn't work on osx
             when we use an alternate stack */
          sig_action.sa_flags = SA_SIGINFO;
      #else
          sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
      #endif

      if (sigaction(SIGSEGV, &sig_action, NULL) != 0) { err(1, "sigaction"); }
      if (sigaction(SIGFPE,  &sig_action, NULL) != 0) { err(1, "sigaction"); }
      if (sigaction(SIGINT,  &sig_action, NULL) != 0) { err(1, "sigaction"); }
      if (sigaction(SIGILL,  &sig_action, NULL) != 0) { err(1, "sigaction"); }
      if (sigaction(SIGTERM, &sig_action, NULL) != 0) { err(1, "sigaction"); }
      if (sigaction(SIGABRT, &sig_action, NULL) != 0) { err(1, "sigaction"); }
    }
  }
#endif

int  divide_by_zero();
func_declspec void call_convention cause_segfault();
void stack_overflow();
void infinite_loop();
void illegal_instruction();
func_declspec void call_convention cause_calamity();

static char const * icky_global_program_name;

#ifdef AS_DLL
func_declspec int call_convention init()
// BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
#else
int main(int argc, char * argv[])
#endif
{

#ifdef AS_DLL
  icky_global_program_name = "todo";
#else
  (void)argc;

  /* store off program path so we can use it later */
  icky_global_program_name = argv[0];
#endif

  set_signal_handler();

#ifndef AS_DLL
  cause_calamity();

  puts("OMG! Nothing bad happend!");
#endif

  return 0;
}

func_declspec void call_convention cause_calamity()
{
  /* uncomment one of the following error conditions to cause a calamity of 
   your choosing! */
  
  // (void)divide_by_zero();
  cause_segfault();
  // assert(false);
  // infinite_loop();
  // illegal_instruction();
  // stack_overflow();
}



int divide_by_zero()
{
  int a = 1;
  int b = 0; 
  return a / b;
}

func_declspec void call_convention cause_segfault()
{
  MessageBox(0, "cause_segfault 1", 0, 0);
//int * p = (int*)0x12345678;
  int * p = (int*)0;
  *p = 0;
  MessageBox(0, "cause_segfault 2", 0, 0);
}

void stack_overflow();
void stack_overflow()
{
  int foo[1000];
  (void)foo;
  stack_overflow();
}

/* break out with ctrl+c to test SIGINT handling */
void infinite_loop()
{
  while(1) {};
}

void illegal_instruction()
{
  /* I couldn't find an easy way to cause this one, so I'm cheating */
  raise(SIGILL);
}
