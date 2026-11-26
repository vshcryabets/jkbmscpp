#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

#include "stateflow.h"

using namespace flow;

// Simple test state type with equality operators
struct SimpleState {
    int value = 0;

    bool operator==(const SimpleState& other) const {
        return value == other.value;
    }

    bool operator!=(const SimpleState& other) const {
        return !(*this == other);
    }
};

// String state for more complex testing
struct StringState {
    std::string text;

    bool operator==(const StringState& other) const {
        return text == other.text;
    }

    bool operator!=(const StringState& other) const {
        return !(*this == other);
    }
};

// Complex state with multiple fields
struct ComplexState {
    int id = 0;
    std::string name;
    bool active = false;

    bool operator==(const ComplexState& other) const {
        return id == other.id && name == other.name && active == other.active;
    }

    bool operator!=(const ComplexState& other) const {
        return !(*this == other);
    }
};

// Test listener implementation
template <typename T>
class TestListener : public StateListener<T> {
public:
    std::atomic<int> call_count{0};
    std::vector<T> received_states;
    mutable std::mutex state_mutex;

    void on_state_changed(const T& state) override {
        call_count++;
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            received_states.push_back(state);
        }
    }

    T last_state() const {
        std::lock_guard<std::mutex> lock(state_mutex);
        if (!received_states.empty()) {
            return received_states.back();
        }
        return T();
    }

    size_t get_received_states_size() const {
        std::lock_guard<std::mutex> lock(state_mutex);
        return received_states.size();
    }
};

// Test unsubscribe listener implementation
template <typename T>
class TestUnsubscribeListener : public UnsubscribeListener<T> {
public:
    int unsubscribe_count = 0;
    int last_unsubscribe_count = 0;
    StateListener<T>* last_unsubscribed_listener = nullptr;

    void onUnsubscribe(StateListener<T>* listener) override {
        unsubscribe_count++;
        last_unsubscribed_listener = listener;
    }

    void onLastUnsubscribe() override {
        last_unsubscribe_count++;
    }
};

TEST_CASE("StateFlowImpl basic construction", "[StateFlowImpl]") {
    SECTION("construct with initial state") {
        SimpleState initial{42};
        StateFlowImpl<SimpleState> flow(initial);
        
        REQUIRE(flow.state().value == 42);
    }

    SECTION("construct with initial state and unsubscribe listener") {
        SimpleState initial{10};
        TestUnsubscribeListener<SimpleState> listener;
        StateFlowImpl<SimpleState> flow(initial, &listener);
        
        REQUIRE(flow.state().value == 10);
    }
}

TEST_CASE("StateFlowImpl state access", "[StateFlowImpl]") {
    StateFlowImpl<SimpleState> flow({0});

    SECTION("state() returns current state") {
        flow.emit({100});
        REQUIRE(flow.state().value == 100);
    }

    SECTION("state() is thread-safe") {
        std::vector<std::thread> threads;
        const int thread_count = 10;
        const int iterations = 100;

        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&flow, i, iterations]() {
                for (int j = 0; j < iterations; ++j) {
                    flow.emit({i * 1000 + j});
                    auto current = flow.state();
                    REQUIRE(current.value >= 0);
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }
    }
}

TEST_CASE("StateFlowImpl subscribe", "[StateFlowImpl]") {
    StateFlowImpl<SimpleState> flow({0});
    TestListener<SimpleState> listener;

    SECTION("subscriber receives current state immediately") {
        flow.emit({42});
        bool result = flow.subscribe(&listener);

        REQUIRE(result);
        REQUIRE(listener.call_count == 1);
        REQUIRE(listener.last_state().value == 42);
    }

    SECTION("subscribe with null listener returns false") {
        bool result = flow.subscribe(nullptr);
        REQUIRE(!result);
        REQUIRE(flow.state().value == 0);
    }

    SECTION("multiple subscribers receive state") {
        TestListener<SimpleState> listener1, listener2, listener3;
        
        REQUIRE(flow.subscribe(&listener1));
        REQUIRE(flow.subscribe(&listener2));
        REQUIRE(flow.subscribe(&listener3));

        REQUIRE(listener1.call_count == 1);
        REQUIRE(listener2.call_count == 1);
        REQUIRE(listener3.call_count == 1);

        flow.emit({99});

        REQUIRE(listener1.call_count == 2);
        REQUIRE(listener2.call_count == 2);
        REQUIRE(listener3.call_count == 2);
        REQUIRE(listener1.last_state().value == 99);
        REQUIRE(listener2.last_state().value == 99);
        REQUIRE(listener3.last_state().value == 99);
    }

    SECTION("subscribe is thread-safe") {
        std::vector<std::thread> threads;
        std::vector<TestListener<SimpleState>> listeners(10);
        const int ops_per_thread = 50;

        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([&flow, &listeners, i, ops_per_thread]() {
                for (int j = 0; j < ops_per_thread; ++j) {
                    bool result = flow.subscribe(&listeners[i]);
                    REQUIRE(result);
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        // All listeners should have received at least the initial state
        for (auto& listener : listeners) {
            REQUIRE(listener.call_count > 0);
        }
    }
}

TEST_CASE("StateFlowImpl unsubscribe", "[StateFlowImpl]") {
    StateFlowImpl<SimpleState> flow({0});
    TestListener<SimpleState> listener;

    SECTION("unsubscribed listener stops receiving updates") {
        flow.subscribe(&listener);
        REQUIRE(listener.call_count == 1);

        flow.emit({1});
        REQUIRE(listener.call_count == 2);

        flow.unsubscribe(&listener);
        flow.emit({2});

        REQUIRE(listener.call_count == 2);
        REQUIRE(listener.last_state().value == 1);
    }

    SECTION("unsubscribe null listener does nothing") {
        flow.unsubscribe(nullptr);
        REQUIRE(flow.state().value == 0);
    }

    SECTION("unsubscribe non-subscribed listener does nothing") {
        TestListener<SimpleState> other;
        flow.subscribe(&listener);
        
        flow.unsubscribe(&other);
        flow.emit({1});

        REQUIRE(listener.call_count == 2);
    }

    SECTION("unsubscribe with unsubscribe listener") {
        TestUnsubscribeListener<SimpleState> unsub_listener;
        StateFlowImpl<SimpleState> flow_with_listener({0}, &unsub_listener);
        
        flow_with_listener.subscribe(&listener);
        flow_with_listener.unsubscribe(&listener);

        REQUIRE(unsub_listener.unsubscribe_count == 1);
        REQUIRE(unsub_listener.last_unsubscribed_listener == &listener);
    }

    SECTION("unsubscribe is thread-safe") {
        std::vector<TestListener<SimpleState>> listeners(10);
        for (auto& l : listeners) {
            flow.subscribe(&l);
        }

        std::vector<std::thread> threads;
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([&flow, &listeners, i]() {
                flow.unsubscribe(&listeners[i]);
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        flow.emit({99});

        // No listeners should receive the update
        for (auto& listener : listeners) {
            REQUIRE(listener.last_state().value != 99);
        }
    }
}

TEST_CASE("StateFlowImpl emit", "[StateFlowImpl]") {
    StateFlowImpl<SimpleState> flow({0});
    TestListener<SimpleState> listener;
    flow.subscribe(&listener);

    SECTION("emit changes state") {
        REQUIRE(flow.state().value == 0);
        
        flow.emit({42});
        REQUIRE(flow.state().value == 42);
    }

    SECTION("emit with same state does not notify") {
        flow.emit({10});
        listener.call_count = 0;  // Reset count

        flow.emit({10});
        REQUIRE(listener.call_count == 0);
    }

    SECTION("emit with different state notifies listeners") {
        listener.call_count = 0;  // Reset after subscription
        
        flow.emit({20});
        REQUIRE(listener.call_count == 1);
        REQUIRE(listener.last_state().value == 20);

        flow.emit({30});
        REQUIRE(listener.call_count == 2);
        REQUIRE(listener.last_state().value == 30);
    }

    SECTION("emit is thread-safe") {
        std::vector<std::thread> threads;
        const int thread_count = 5;
        const int iterations = 100;

        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&flow, i, iterations]() {
                for (int j = 0; j < iterations; ++j) {
                    flow.emit({i * iterations + j});
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        REQUIRE(listener.get_received_states_size() >= 1);
    }

    SECTION("emit with complex state") {
        StateFlowImpl<ComplexState> complex_flow({1, "initial", false});
        TestListener<ComplexState> complex_listener;
        complex_flow.subscribe(&complex_listener);

        ComplexState new_state{2, "updated", true};
        complex_flow.emit(new_state);

        REQUIRE(complex_listener.last_state().id == 2);
        REQUIRE(complex_listener.last_state().name == "updated");
        REQUIRE(complex_listener.last_state().active == true);
    }
}

TEST_CASE("StateFlowImpl update", "[StateFlowImpl]") {
    StateFlowImpl<SimpleState> flow({0});
    TestListener<SimpleState> listener;
    flow.subscribe(&listener);

    SECTION("update changes state") {
        bool changed = flow.update([](const SimpleState& current) {
            return SimpleState{current.value + 1};
        });

        REQUIRE(changed);
        REQUIRE(flow.state().value == 1);
    }

    SECTION("update returns false if state unchanged") {
        listener.call_count = 0;  // Reset after subscription

        bool changed = flow.update([](const SimpleState& current) {
            return current;  // Return same state
        });

        REQUIRE(!changed);
        REQUIRE(listener.call_count == 0);
    }

    SECTION("update returns true if state changed") {
        listener.call_count = 0;  // Reset

        bool changed = flow.update([](const SimpleState& current) {
            return SimpleState{current.value + 10};
        });

        REQUIRE(changed);
        REQUIRE(listener.call_count == 1);
    }

    SECTION("update with function parameter") {
        bool changed = flow.update([](const SimpleState& current) {
            return SimpleState{current.value * 2 + 1};
        });

        REQUIRE(changed);
        REQUIRE(flow.state().value == 1);

        flow.emit({5});
        listener.call_count = 0;

        changed = flow.update([](const SimpleState& current) {
            return SimpleState{current.value * 2};
        });

        REQUIRE(changed);
        REQUIRE(flow.state().value == 10);
        REQUIRE(flow.state().value == 10);
    }

    SECTION("update is atomic") {
        std::atomic<int> update_count{0};

        flow.update([&](const SimpleState& current) {
            update_count++;
            return SimpleState{current.value + 1};
        });

        REQUIRE(update_count == 1);
        REQUIRE(flow.state().value == 1);
    }

    SECTION("update with lambda capturing state") {
        int multiplier = 3;

        flow.emit({10});
        listener.call_count = 0;

        bool changed = flow.update([multiplier](const SimpleState& current) {
            return SimpleState{current.value * multiplier};
        });

        REQUIRE(changed);
        REQUIRE(flow.state().value == 30);
    }

    SECTION("update is thread-safe") {
        std::vector<std::thread> threads;
        const int thread_count = 4;
        const int iterations = 100;

        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&flow, iterations]() {
                for (int j = 0; j < iterations; ++j) {
                    flow.update([](const SimpleState& current) {
                        return SimpleState{current.value + 1};
                    });
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        REQUIRE(flow.state().value == thread_count * iterations);
    }

    SECTION("update with complex state") {
        StateFlowImpl<ComplexState> complex_flow({1, "test", false});
        TestListener<ComplexState> complex_listener;
        complex_flow.subscribe(&complex_listener);

        bool changed = complex_flow.update([](const ComplexState& current) {
            ComplexState next = current;
            next.id += 1;
            next.name += "_updated";
            next.active = true;
            return next;
        });

        REQUIRE(changed);
        REQUIRE(complex_flow.state().id == 2);
        REQUIRE(complex_flow.state().name == "test_updated");
        REQUIRE(complex_flow.state().active == true);
    }
}

TEST_CASE("StateFlowImpl UnsubscribeListener", "[StateFlowImpl]") {
    StateFlowImpl<SimpleState> flow({0});
    TestUnsubscribeListener<SimpleState> unsub_listener;
    StateFlowImpl<SimpleState> flow_with_listener({0}, &unsub_listener);
    TestListener<SimpleState> listener1, listener2;

    SECTION("onUnsubscribe is called when listener unsubscribes") {
        REQUIRE(flow_with_listener.subscribe(&listener1));
        
        REQUIRE(unsub_listener.unsubscribe_count == 0);

        flow_with_listener.unsubscribe(&listener1);

        REQUIRE(unsub_listener.unsubscribe_count == 1);
        REQUIRE(unsub_listener.last_unsubscribed_listener == &listener1);
    }

    SECTION("onLastUnsubscribe is called when last listener unsubscribes") {
        REQUIRE(flow_with_listener.subscribe(&listener1));
        REQUIRE(flow_with_listener.subscribe(&listener2));

        flow_with_listener.unsubscribe(&listener1);
        REQUIRE(unsub_listener.last_unsubscribe_count == 0);

        flow_with_listener.unsubscribe(&listener2);
        REQUIRE(unsub_listener.last_unsubscribe_count == 1);
    }

    SECTION("onLastUnsubscribe not called if other listeners remain") {
        REQUIRE(flow_with_listener.subscribe(&listener1));
        REQUIRE(flow_with_listener.subscribe(&listener2));

        flow_with_listener.unsubscribe(&listener1);
        REQUIRE(unsub_listener.last_unsubscribe_count == 0);
    }

    SECTION("unsubscribe listener with null does nothing") {
        StateFlowImpl<SimpleState> flow_null({0}, nullptr);
        TestListener<SimpleState> listener;
        
        REQUIRE(flow_null.subscribe(&listener));
        flow_null.unsubscribe(&listener);
        
        REQUIRE(listener.call_count >= 1);
    }
}

TEST_CASE("StateFlowImpl with different state types", "[StateFlowImpl]") {
    SECTION("string state") {
        StateFlowImpl<StringState> flow({""});
        TestListener<StringState> listener;
        REQUIRE(flow.subscribe(&listener));

        flow.emit({"hello"});
        REQUIRE(listener.last_state().text == "hello");

        flow.emit({"world"});
        REQUIRE(listener.last_state().text == "world");

        // Same string should not trigger notification
        listener.call_count = 0;
        flow.emit({"world"});
        REQUIRE(listener.call_count == 0);
    }

    SECTION("complex state with multiple fields") {
        StateFlowImpl<ComplexState> flow({0, "", false});
        TestListener<ComplexState> listener;
        REQUIRE(flow.subscribe(&listener));

        ComplexState state1{1, "first", false};
        flow.emit(state1);

        REQUIRE(listener.last_state().id == 1);
        REQUIRE(listener.last_state().name == "first");
        REQUIRE(listener.last_state().active == false);

        // Change only id, should trigger notification
        listener.call_count = 0;
        ComplexState state2{2, "first", false};
        flow.emit(state2);
        REQUIRE(listener.call_count == 1);

        // All same, should not trigger notification
        listener.call_count = 0;
        flow.emit(state2);
        REQUIRE(listener.call_count == 0);
    }
}

TEST_CASE("StateFlowImpl ordering and sequencing", "[StateFlowImpl]") {
    StateFlowImpl<SimpleState> flow({0});
    TestListener<SimpleState> listener;
    REQUIRE(flow.subscribe(&listener));

    SECTION("state changes are delivered in order") {
        const int max_state = 100;
        
        for (int i = 1; i <= max_state; ++i) {
            flow.emit({i});
        }

        size_t size = listener.get_received_states_size();
        REQUIRE(size == max_state + 1);  // +1 for initial state

        // Check ordering (skipping initial subscription state)
        {
            std::lock_guard<std::mutex> lock(listener.state_mutex);
            for (size_t i = 1; i < listener.received_states.size(); ++i) {
                REQUIRE(listener.received_states[i].value == static_cast<int>(i));
            }
        }
    }

    SECTION("rapid sequential updates") {
        listener.call_count = 0;
        
        for (int i = 0; i < 50; ++i) {
            flow.update([](const SimpleState& current) {
                return SimpleState{current.value + 1};
            });
        }

        REQUIRE(flow.state().value == 50);
        REQUIRE(listener.call_count == 50);
    }
}

TEST_CASE("StateFlowImpl edge cases", "[StateFlowImpl]") {
    SECTION("subscribe then immediately unsubscribe") {
        StateFlowImpl<SimpleState> flow({0});
        TestListener<SimpleState> listener;

        REQUIRE(flow.subscribe(&listener));
        flow.unsubscribe(&listener);

        // Should have received initial state only
        REQUIRE(listener.call_count == 1);

        flow.emit({99});
        REQUIRE(listener.call_count == 1);
    }

    SECTION("re-subscribe after unsubscribe") {
        StateFlowImpl<SimpleState> flow({0});
        TestListener<SimpleState> listener;

        REQUIRE(flow.subscribe(&listener));
        REQUIRE(listener.call_count == 1);

        flow.unsubscribe(&listener);

        flow.emit({10});
        REQUIRE(listener.call_count == 1);  // No new notification

        REQUIRE(flow.subscribe(&listener));
        REQUIRE(listener.call_count == 2);  // Receives current state
        REQUIRE(listener.last_state().value == 10);
    }

    SECTION("multiple state updates before listener subscribes") {
        StateFlowImpl<SimpleState> flow({0});
        TestListener<SimpleState> listener;

        flow.emit({1});
        flow.emit({2});
        flow.emit({3});

        REQUIRE(flow.subscribe(&listener));

        REQUIRE(listener.call_count == 1);
        REQUIRE(listener.last_state().value == 3);  // Gets current state only
    }
}

TEST_CASE("StateFlowImpl stress test", "[StateFlowImpl]") {
    SECTION("concurrent emit calls") {
        StateFlowImpl<SimpleState> flow({0});
        const int thread_count = 4;
        const int iterations = 50;

        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&flow, i, iterations]() {
                for (int j = 0; j < iterations; ++j) {
                    flow.emit({i * iterations + j + 1});
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        REQUIRE(flow.state().value > 0);
    }

    SECTION("concurrent update calls") {
        StateFlowImpl<SimpleState> flow({0});
        const int thread_count = 4;
        const int iterations = 25;

        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&flow, iterations]() {
                for (int j = 0; j < iterations; ++j) {
                    flow.update([](const SimpleState& current) {
                        return SimpleState{current.value + 1};
                    });
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        REQUIRE(flow.state().value == thread_count * iterations);
    }
}
