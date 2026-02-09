#pragma once

#include <atomic>
#include <memory>

// 单生产者单消费者无锁队列
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::atomic<Node*> next;
        
        Node() : next(nullptr) {}
    };
    
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    
    Node* pop_head() {
        Node* const old_head = head.load(std::memory_order_relaxed);
        if (old_head == tail.load(std::memory_order_acquire)) {
            return nullptr;
        }
        head.store(old_head->next.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return old_head;
    }
    
public:
    LockFreeQueue() : head(new Node()), tail(head.load()) {}
    
    ~LockFreeQueue() {
        while (Node* const old_head = head.load()) {
            head.store(old_head->next);
            delete old_head;
        }
    }
    
    void push(T value) {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(value)));
        Node* p = new Node();
        Node* const old_tail = tail.exchange(p, std::memory_order_acq_rel);
        old_tail->data.swap(new_data);
        old_tail->next.store(p, std::memory_order_release);
    }
    
    bool pop(T& value) {
        Node* old_head = pop_head();
        if (!old_head) {
            return false;
        }
        std::shared_ptr<T> const old_data = std::move(old_head->data);
        delete old_head;
        value = std::move(*old_data);
        return true;
    }
    
    bool empty() const {
        return head.load(std::memory_order_relaxed) == tail.load(std::memory_order_acquire);
    }
};