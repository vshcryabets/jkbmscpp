#pragma once

#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>
#include <type_traits>
#include <functional>

namespace flow {

    /// @brief Type trait to check if T has operator== and operator!=
    /// Detects whether type T supports equality comparison operators
    template <typename T, typename = void>
    struct has_equality_operators : std::false_type {};

    /// @brief Specialization for types that have operator== and operator!=
    template <typename T>
    struct has_equality_operators<T, std::void_t<
        decltype(std::declval<const T&>() == std::declval<const T&>()),
        decltype(std::declval<const T&>() != std::declval<const T&>())
    >> : std::true_type {};

    /// @brief Helper variable template for has_equality_operators
    template <typename T>
    inline constexpr bool has_equality_operators_v = has_equality_operators<T>::value;

    /// @brief Type trait to check if T has operator== (legacy alias)
    /// @deprecated Use has_equality_operators instead
    template <typename T>
    inline constexpr bool has_operator_equal_v = has_equality_operators_v<T>;

    /// @brief Observer interface for state changes
    /// 
    /// Template parameter requirements:
    /// - T must be copy-constructible
    /// - T must support operator== and operator!= for equality comparison
    /// 
    /// @tparam T The state type to observe. Must define operator== and operator!=.
    template <typename T>
    class StateListener {
    public:
        /// @brief Virtual destructor for proper cleanup
        virtual ~StateListener() = default;

        /// @brief Called when the observed state changes
        /// 
        /// This method is invoked each time a new state is emitted that differs
        /// from the previous state (determined by operator==).
        /// 
        /// @param state The new state value
        virtual void on_state_changed(const T& state) = 0;
    };

    /// @brief Observer interface for subscription lifecycle events
    /// 
    /// UnsubscribeListener provides callbacks for tracking when listeners
    /// unsubscribe from a StateFlow. Useful for resource cleanup and
    /// tracking subscription state.
    /// 
    /// @tparam T The state type (not used in callbacks, required for template consistency)
    template <typename T>
    class UnsubscribeListener {
    public:
        /// @brief Virtual destructor for proper cleanup
        virtual ~UnsubscribeListener() = default;

        /// @brief Called when a listener unsubscribes
        /// 
        /// This method is invoked when a StateListener unsubscribes from the StateFlow.
        /// 
        /// @param listener Pointer to the listener that unsubscribed
        virtual void onUnsubscribe(StateListener<T>* listener) = 0;

        /// @brief Called when the last listener unsubscribes
        /// 
        /// This method is invoked when the last StateListener is removed from the StateFlow,
        /// meaning there are no more StateListener subscribers (though there may still be
        /// lambda-based subscribers).
        virtual void onLastUnsubscribe() = 0;
    };

    /// @brief Thread-safe reactive state container with observer pattern
    /// 
    /// StateFlow is a reactive state holder that notifies registered listeners
    /// whenever the state changes. It uses operator== and operator!= to determine 
    /// if a state change has occurred.
    /// 
    /// Features:
    /// - Thread-safe state management with mutex protection
    /// - Observer pattern for reactive updates
    /// - State change detection via operator== and operator!=
    /// - Support for functional state updates
    /// - Copy-on-subscribe pattern (new subscribers receive current state immediately)
    /// 
    /// Usage example:
    /// @code
    /// struct MyState {
    ///     int value;
    ///     bool operator==(const MyState& other) const {
    ///         return value == other.value;
    ///     }
    ///     bool operator!=(const MyState& other) const {
    ///         return !(*this == other);
    ///     }
    /// };
    /// 
    /// flow::StateFlow<MyState> stateFlow({0});
    /// 
    /// class MyListener : public flow::StateListener<MyState> {
    ///     void on_state_changed(const MyState& state) override {
    ///         std::cout << "New value: " << state.value << std::endl;
    ///     }
    /// };
    /// 
    /// MyListener listener;
    /// stateFlow.subscribe(&listener);        // listener receives current state
    /// stateFlow.emit({42});                  // listener is notified with new state
    /// stateFlow.unsubscribe(&listener);      // stop observing
    /// @endcode
    /// 
    /// Template parameter requirements:
    /// - T must be copy-constructible
    /// - T must define operator== and operator!= for equality comparison (compile-time checked)
    /// 
    /// @tparam T The state type. Must define operator== and operator!= for change detection.
    template <typename T>
    class StateFlow {
        static_assert(has_equality_operators_v<T>,
            "StateFlow template parameter T must define both operator== and operator!= for equality comparison");

    public:
        /// @brief Alias for the state type
        using StateType = T;

        /// @brief Internal storage for shared state and listeners
        /// 
        /// Separates state and listeners into distinct protected regions to minimize
        /// lock contention. Uses shared_ptr to allow multiple StateFlow instances
        /// to share the same underlying state (if needed).
        struct Storage {
            /// @brief Current state value
            StateType state;

            /// @brief Mutex protecting state reads/writes
            mutable std::mutex state_mutex;

            /// @brief List of registered state listeners
            std::vector<StateListener<StateType>*> listeners;
            
            /// @brief Unsubscribe listener for lifecycle events
            UnsubscribeListener<StateType>* unsubscribe_listener = nullptr;

            /// @brief Mutex protecting listener list modifications
            mutable std::mutex listeners_mutex;
        };

        /// @brief Constructs a StateFlow with an initial state
        /// 
        /// @param initial_state The initial state value
        explicit StateFlow(const StateType& initial_state)
            : storage_(std::make_shared<Storage>())
        {
            storage_->state = initial_state;
        }

        /// @brief Constructs a StateFlow with an initial state and unsubscribe listener
        /// 
        /// @param initial_state The initial state value
        /// @param unsubscribe_listener Listener for subscription lifecycle events (can be nullptr)
        StateFlow(const StateType& initial_state, UnsubscribeListener<StateType>* unsubscribe_listener)
            : storage_(std::make_shared<Storage>())
        {
            storage_->state = initial_state;
            storage_->unsubscribe_listener = unsubscribe_listener;
        }

        /// @brief Returns the current state value
        /// 
        /// Thread-safe read of the current state. The returned value is a copy
        /// and subsequent modifications to the original state will not affect it.
        /// 
        /// @return A copy of the current state
        StateType state() const noexcept
        {
            std::lock_guard lock(storage_->state_mutex);
            return storage_->state;
        }

        /// @brief Subscribes a listener to state changes
        /// 
        /// Registers a listener to receive notifications when the state changes.
        /// The listener is immediately notified with the current state (even if
        /// it hasn't changed since subscription).
        /// 
        /// Thread-safe and safe to call concurrently with other operations.
        /// Does nothing if listener is null.
        /// 
        /// @param listener Pointer to the listener to subscribe. Must not be null
        ///                 and must remain valid until unsubscribe() is called.
        void subscribe(StateListener<StateType>* listener)
        {
            if (!listener) return;
            StateType current_state;
            {
                std::lock_guard lock(storage_->listeners_mutex);
                storage_->listeners.push_back(listener);
            }
            {
                std::lock_guard lock(storage_->state_mutex);
                current_state = storage_->state;
            }
            listener->on_state_changed(current_state);
        }

        /// @brief Unsubscribes a listener from state changes
        /// 
        /// Removes a previously subscribed listener. The listener will no longer
        /// receive state change notifications after this call.
        /// 
        /// Thread-safe and safe to call concurrently with other operations.
        /// Does nothing if the listener is not currently subscribed.
        /// 
        /// @param listener Pointer to the listener to unsubscribe
        void unsubscribe(StateListener<StateType>* listener)
        {
            bool was_last_listener = false;
            {
                std::lock_guard lock(storage_->listeners_mutex);
                storage_->listeners.erase(
                    std::remove_if(storage_->listeners.begin(), storage_->listeners.end(), 
                    [&](const StateListener<StateType>* existing) {
                        return existing == listener;
                    }),
                    storage_->listeners.end());
                
                // Check if this was the last listener
                was_last_listener = storage_->listeners.empty();
            }
            
            // Notify unsubscribe listener (outside the lock to avoid deadlocks)
            if (storage_->unsubscribe_listener) {
                storage_->unsubscribe_listener->onUnsubscribe(listener);
                if (was_last_listener) {
                    storage_->unsubscribe_listener->onLastUnsubscribe();
                }
            }
        }

        /// @brief Emits a new state and notifies all listeners if it differs from current state
        /// 
        /// Atomically updates the state and notifies all registered listeners.
        /// Listeners are only notified if the new state differs from the current state
        /// (determined by operator==).
        /// 
        /// Thread-safe and safe to call concurrently from multiple threads.
        /// 
        /// @param new_state The new state value to emit
        void emit(const StateType& new_state)
        {
            bool has_changes = false;
            {
                std::lock_guard lock(storage_->state_mutex);
                if (new_state != storage_->state)
                {
                    storage_->state = new_state;
                    has_changes = true;
                }
            }

            if (has_changes)
            {
                notify_state_changed(new_state);
            }
        }

        /// @brief Updates state using a function and notifies listeners if changed
        /// 
        /// Applies an updater function to the current state to produce a new state.
        /// The state is atomically updated and listeners are notified if the new
        /// state differs from the previous state.
        /// 
        /// This is useful for state transformations where the new state depends on
        /// the current state, ensuring atomicity without external synchronization.
        /// 
        /// Thread-safe and safe to call concurrently from multiple threads.
        /// 
        /// Example:
        /// @code
        /// struct Counter {
        ///     int count;
        ///     bool operator==(const Counter& other) const {
        ///         return count == other.count;
        ///     }
        /// };
        /// 
        /// flow::StateFlow<Counter> flow({0});
        /// 
        /// bool changed = flow.update([](const Counter& current) {
        ///     return Counter{current.count + 1};
        /// });
        /// @endcode
        /// 
        /// @tparam Updater A callable that takes const StateType& and returns StateType
        /// @param updater Function to update the state. Must have signature:
        ///                StateType(const StateType&)
        /// @return true if the state was changed, false if the updater produced
        ///         the same state (as determined by operator==)
        template <typename Updater>
        bool update(Updater&& updater)
        {
            bool has_changes = false;
            std::optional<StateType> snapshot;
            {
                std::lock_guard lock(storage_->state_mutex);
                const StateType next_state = updater(storage_->state);
                if (next_state != storage_->state)
                {
                    storage_->state = next_state;
                    snapshot = storage_->state; 
                    has_changes = true;
                }
            }

            if (has_changes && snapshot.has_value())
            {
                notify_state_changed(snapshot.value());
            }

            return has_changes;
        }

    private:
        /// @brief Shared storage for state and listeners
        std::shared_ptr<Storage> storage_;

        /// @brief Notifies all listeners of a state change
        /// 
        /// Makes a copy of the listener list under lock, then notifies each listener
        /// outside the lock to minimize hold time and avoid deadlocks.
        /// 
        /// @param snapshot The new state to send to listeners
        void notify_state_changed(const StateType& snapshot) const
        {
            std::vector<StateListener<StateType>*> listeners_copy;
            
            {
                std::lock_guard lock(storage_->listeners_mutex);
                listeners_copy = storage_->listeners;
            }

            // Notify StateListener subscribers
            for (const auto& listener : listeners_copy)
            {
                if (listener)
                {
                    listener->on_state_changed(snapshot);
                }
            }
        }
    };

}