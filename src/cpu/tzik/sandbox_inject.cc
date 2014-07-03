#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>

namespace {

template <typename T, size_t N>
constexpr size_t arraysize(T(&)[N]) {
  return N;
}

void print_syscall_name(int syscall_number);

__attribute__((constructor))
void apply_seccomp_filter() {

  sock_filter filter[] = {
    // Load |arch| to |A|.
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(seccomp_data, arch))),

    // Disallow non-x86_64.
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

    // Load |nr|, syscall number, to |A|.
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(seccomp_data, nr))),

#define ALLOW_SYSCALL(x)                                \
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_##x, 0, 1), \
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)

    ALLOW_SYSCALL(read),
    ALLOW_SYSCALL(write),
    ALLOW_SYSCALL(close),
    ALLOW_SYSCALL(fstat),

    ALLOW_SYSCALL(exit),
    ALLOW_SYSCALL(exit_group),

    ALLOW_SYSCALL(brk),
    ALLOW_SYSCALL(mmap),
    ALLOW_SYSCALL(munmap),
    ALLOW_SYSCALL(mprotect),
    ALLOW_SYSCALL(madvise),

    ALLOW_SYSCALL(futex),
    ALLOW_SYSCALL(clone),
    ALLOW_SYSCALL(set_robust_list),

    ALLOW_SYSCALL(rt_sigaction),
#undef ALLOW_SYSCALL

    // Disallow others.
    // Store negated syscall number to |errno| and signal SIGSYS.
    BPF_STMT(BPF_ALU | BPF_NEG, 0),
    BPF_STMT(BPF_ALU | BPF_AND | BPF_K, SECCOMP_RET_DATA),
    BPF_STMT(BPF_ALU | BPF_OR | BPF_K, SECCOMP_RET_TRAP),
    BPF_STMT(BPF_RET | BPF_A, 0),
  };

  sock_fprog program = {
    .len = arraysize(filter),
    .filter = filter
  };

  struct sigaction action = {
    .sa_sigaction = [](int, siginfo_t* siginfo, void*) {
      int16_t syscall_number = -static_cast<int16_t>(siginfo->si_errno);
      const char msg[] = "Disallowed syscall: ";
      write(2, msg, sizeof(msg));
      print_syscall_name(syscall_number);

      write(2, " (", 2);
      char buf[] = "     )\n";
      int i = 5;
      while (syscall_number) {
        buf[--i] = '0' + (syscall_number % 10);
        syscall_number /= 10;
      }
      write(2, buf + i, 5 - i + 2);
    },
    .sa_flags = SA_SIGINFO,
  };
  sigaction(SIGSYS, &action, nullptr);

  if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0 ||
      prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &program, 0, 0) < 0) {
    perror("prctl");
    abort();
  }
}

void print_syscall_name(int syscall_number) {
#define CASE(x)                                 \
    case __NR_ ## x: {                          \
      const char name[] = #x;                   \
          write(2, name, sizeof(name));         \
          break;                                \
    }
  switch (syscall_number) {
    CASE(read);
    CASE(write);
    CASE(open);
    CASE(close);
    CASE(stat);
    CASE(fstat);
    CASE(lstat);
    CASE(poll);
    CASE(lseek);
    CASE(mmap);
    CASE(mprotect);
    CASE(munmap);
    CASE(brk);
    CASE(rt_sigaction);
    CASE(rt_sigprocmask);
    CASE(rt_sigreturn);
    CASE(ioctl);
    CASE(pread64);
    CASE(pwrite64);
    CASE(readv);
    CASE(writev);
    CASE(access);
    CASE(pipe);
    CASE(select);
    CASE(sched_yield);
    CASE(mremap);
    CASE(msync);
    CASE(mincore);
    CASE(madvise);
    CASE(shmget);
    CASE(shmat);
    CASE(shmctl);
    CASE(dup);
    CASE(dup2);
    CASE(pause);
    CASE(nanosleep);
    CASE(getitimer);
    CASE(alarm);
    CASE(setitimer);
    CASE(getpid);
    CASE(sendfile);
    CASE(socket);
    CASE(connect);
    CASE(accept);
    CASE(sendto);
    CASE(recvfrom);
    CASE(sendmsg);
    CASE(recvmsg);
    CASE(shutdown);
    CASE(bind);
    CASE(listen);
    CASE(getsockname);
    CASE(getpeername);
    CASE(socketpair);
    CASE(setsockopt);
    CASE(getsockopt);
    CASE(clone);
    CASE(fork);
    CASE(vfork);
    CASE(execve);
    CASE(exit);
    CASE(wait4);
    CASE(kill);
    CASE(uname);
    CASE(semget);
    CASE(semop);
    CASE(semctl);
    CASE(shmdt);
    CASE(msgget);
    CASE(msgsnd);
    CASE(msgrcv);
    CASE(msgctl);
    CASE(fcntl);
    CASE(flock);
    CASE(fsync);
    CASE(fdatasync);
    CASE(truncate);
    CASE(ftruncate);
    CASE(getdents);
    CASE(getcwd);
    CASE(chdir);
    CASE(fchdir);
    CASE(rename);
    CASE(mkdir);
    CASE(rmdir);
    CASE(creat);
    CASE(link);
    CASE(unlink);
    CASE(symlink);
    CASE(readlink);
    CASE(chmod);
    CASE(fchmod);
    CASE(chown);
    CASE(fchown);
    CASE(lchown);
    CASE(umask);
    CASE(gettimeofday);
    CASE(getrlimit);
    CASE(getrusage);
    CASE(sysinfo);
    CASE(times);
    CASE(ptrace);
    CASE(getuid);
    CASE(syslog);
    CASE(getgid);
    CASE(setuid);
    CASE(setgid);
    CASE(geteuid);
    CASE(getegid);
    CASE(setpgid);
    CASE(getppid);
    CASE(getpgrp);
    CASE(setsid);
    CASE(setreuid);
    CASE(setregid);
    CASE(getgroups);
    CASE(setgroups);
    CASE(setresuid);
    CASE(getresuid);
    CASE(setresgid);
    CASE(getresgid);
    CASE(getpgid);
    CASE(setfsuid);
    CASE(setfsgid);
    CASE(getsid);
    CASE(capget);
    CASE(capset);
    CASE(rt_sigpending);
    CASE(rt_sigtimedwait);
    CASE(rt_sigqueueinfo);
    CASE(rt_sigsuspend);
    CASE(sigaltstack);
    CASE(utime);
    CASE(mknod);
    CASE(uselib);
    CASE(personality);
    CASE(ustat);
    CASE(statfs);
    CASE(fstatfs);
    CASE(sysfs);
    CASE(getpriority);
    CASE(setpriority);
    CASE(sched_setparam);
    CASE(sched_getparam);
    CASE(sched_setscheduler);
    CASE(sched_getscheduler);
    CASE(sched_get_priority_max);
    CASE(sched_get_priority_min);
    CASE(sched_rr_get_interval);
    CASE(mlock);
    CASE(munlock);
    CASE(mlockall);
    CASE(munlockall);
    CASE(vhangup);
    CASE(modify_ldt);
    CASE(pivot_root);
    CASE(_sysctl);
    CASE(prctl);
    CASE(arch_prctl);
    CASE(adjtimex);
    CASE(setrlimit);
    CASE(chroot);
    CASE(sync);
    CASE(acct);
    CASE(settimeofday);
    CASE(mount);
    CASE(umount2);
    CASE(swapon);
    CASE(swapoff);
    CASE(reboot);
    CASE(sethostname);
    CASE(setdomainname);
    CASE(iopl);
    CASE(ioperm);
    CASE(create_module);
    CASE(init_module);
    CASE(delete_module);
    CASE(get_kernel_syms);
    CASE(query_module);
    CASE(quotactl);
    CASE(nfsservctl);
    CASE(getpmsg);
    CASE(putpmsg);
    CASE(afs_syscall);
    CASE(tuxcall);
    CASE(security);
    CASE(gettid);
    CASE(readahead);
    CASE(setxattr);
    CASE(lsetxattr);
    CASE(fsetxattr);
    CASE(getxattr);
    CASE(lgetxattr);
    CASE(fgetxattr);
    CASE(listxattr);
    CASE(llistxattr);
    CASE(flistxattr);
    CASE(removexattr);
    CASE(lremovexattr);
    CASE(fremovexattr);
    CASE(tkill);
    CASE(time);
    CASE(futex);
    CASE(sched_setaffinity);
    CASE(sched_getaffinity);
    CASE(set_thread_area);
    CASE(io_setup);
    CASE(io_destroy);
    CASE(io_getevents);
    CASE(io_submit);
    CASE(io_cancel);
    CASE(get_thread_area);
    CASE(lookup_dcookie);
    CASE(epoll_create);
    CASE(epoll_ctl_old);
    CASE(epoll_wait_old);
    CASE(remap_file_pages);
    CASE(getdents64);
    CASE(set_tid_address);
    CASE(restart_syscall);
    CASE(semtimedop);
    CASE(fadvise64);
    CASE(timer_create);
    CASE(timer_settime);
    CASE(timer_gettime);
    CASE(timer_getoverrun);
    CASE(timer_delete);
    CASE(clock_settime);
    CASE(clock_gettime);
    CASE(clock_getres);
    CASE(clock_nanosleep);
    CASE(exit_group);
    CASE(epoll_wait);
    CASE(epoll_ctl);
    CASE(tgkill);
    CASE(utimes);
    CASE(vserver);
    CASE(mbind);
    CASE(set_mempolicy);
    CASE(get_mempolicy);
    CASE(mq_open);
    CASE(mq_unlink);
    CASE(mq_timedsend);
    CASE(mq_timedreceive);
    CASE(mq_notify);
    CASE(mq_getsetattr);
    CASE(kexec_load);
    CASE(waitid);
    CASE(add_key);
    CASE(request_key);
    CASE(keyctl);
    CASE(ioprio_set);
    CASE(ioprio_get);
    CASE(inotify_init);
    CASE(inotify_add_watch);
    CASE(inotify_rm_watch);
    CASE(migrate_pages);
    CASE(openat);
    CASE(mkdirat);
    CASE(mknodat);
    CASE(fchownat);
    CASE(futimesat);
    CASE(newfstatat);
    CASE(unlinkat);
    CASE(renameat);
    CASE(linkat);
    CASE(symlinkat);
    CASE(readlinkat);
    CASE(fchmodat);
    CASE(faccessat);
    CASE(pselect6);
    CASE(ppoll);
    CASE(unshare);
    CASE(set_robust_list);
    CASE(get_robust_list);
    CASE(splice);
    CASE(tee);
    CASE(sync_file_range);
    CASE(vmsplice);
    CASE(move_pages);
    CASE(utimensat);
    CASE(epoll_pwait);
    CASE(signalfd);
    CASE(timerfd_create);
    CASE(eventfd);
    CASE(fallocate);
    CASE(timerfd_settime);
    CASE(timerfd_gettime);
    CASE(accept4);
    CASE(signalfd4);
    CASE(eventfd2);
    CASE(epoll_create1);
    CASE(dup3);
    CASE(pipe2);
    CASE(inotify_init1);
    CASE(preadv);
    CASE(pwritev);
    CASE(rt_tgsigqueueinfo);
    CASE(perf_event_open);
    CASE(recvmmsg);
    CASE(fanotify_init);
    CASE(fanotify_mark);
    CASE(prlimit64);
    CASE(name_to_handle_at);
    CASE(open_by_handle_at);
    CASE(clock_adjtime);
    CASE(syncfs);
    CASE(sendmmsg);
    CASE(setns);
    CASE(getcpu);
    CASE(process_vm_readv);
    CASE(process_vm_writev);
    CASE(kcmp);
    CASE(finit_module);
    default: {
      const char name[] = "unknown";
      write(2, name, sizeof(name));
      break;
    }
  }
#undef CASE
}

}  // namespace
