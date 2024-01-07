void loop() {
    while (!stop) {
        // 取得下次定时任务的时间，与设定time_out去比较大值，即若下次定时任务超过1s就取下次定时任务为超时
        int time_out = Max(1000, getNextTimerCallback());
        // 调用Epoll等待时间发生，超时时间为time_out
        int rt = epoll_wait(epfd, fds, ..., time_out);
        if (rt < 0) {
            // epoll 调用失败
        } else {
            if (rt > 0) {
                foreach (fd in fds) {
                    tasks.push(fd); // 添加任务到执行队列
                }
            }
        }

        foreach (task in tasks) {
            task();
        }
    }
}