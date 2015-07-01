//
//  main.cpp
//  ThreadSample
//
//  Created by pebble8888 on 2015/06/30.
//  Copyright (c) 2015年 pebble8888. All rights reserved.
//

#include <thread>
#include <mutex>
#include <queue>
#include <unistd.h> // usleep

using namespace std;

mutex print_mutex;  // printf()呼び出し排他用
mutex queue_mutex;  // v_queue排他アクセス用
queue<int> v_queue; // データキュー
condition_variable ready_cond; // 条件変数

void worker( void )
{
    while( true ){
        int data;
        {
            // unique_lockではコンストラクタでロックを取得し、デストラクタでロックを解除する。
            // また、明示的にロック、アンロックが可能である。
            unique_lock<mutex> lk(queue_mutex);
            // キューにデータがない場合はキューにデータが追加されたことが
            // 通知されるまで待つ。CPUを余分に消費することがない。
            while( v_queue.empty() ){
                // waitを呼ぶ前にはlkがロック状態でなければならない。
                // C++11でもspurious wakeupの問題があることに注意。
                // ここでready_cond.notify_one()が呼ばれるまでブロックする。
                // ready_cond.notify_one()が呼ばれると、再びロックを取得した状態で、この関数から抜ける。
                ready_cond.wait(lk);
            }
            //
            data = v_queue.front();
            v_queue.pop();
        }
        lock_guard<mutex> l(print_mutex);
        printf( "%p %d\n", this_thread::get_id(), data );
        fflush( stdout );
    }
}

int main(int argc, const char * argv[])
{
    thread a(worker);
    thread b(worker);
    int i = 0;
    while( true ){
        // キューにデータを追加する
        {
            lock_guard<mutex> l(queue_mutex);
            v_queue.push( i++ );
        }
        // ready_cond変数でwaitしているスレッドのどれか一つのロックを解除する。
        ready_cond.notify_one();
        // 1sec待つ
        usleep( 1000000 );
    }
    return 0;
}
