#pragma once

#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>

namespace flow {

    template <typename T>
    class StateListener {
    public:
        virtual ~StateListener() = default;
        virtual void on_state_changed(const T& state) = 0;
    };

    template <typename T>
    class StateFlow {
    public:
        using StateType = T;

        struct Storage {
            StateType state;
            mutable std::mutex state_mutex;
            std::vector<StateListener<StateType>*> listeners;
            mutable std::mutex listeners_mutex;
        };

        explicit StateFlow(const StateType& initial_state)
            : storage_(std::make_shared<Storage>())
        {
            storage_->state = initial_state;
        }

        StateType state() const noexcept
        {
            std::lock_guard lock(storage_->state_mutex);
            return storage_->state;
        }

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

        void unsubscribe(StateListener<StateType>* listener)
        {
            std::lock_guard lock(storage_->listeners_mutex);
            storage_->listeners.erase(
                std::remove_if(storage_->listeners.begin(), storage_->listeners.end(), 
                [&](const StateListener<StateType>* existing) {
                    return existing == listener;
                }),
                storage_->listeners.end());
        }

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
        std::shared_ptr<Storage> storage_;

        void notify_state_changed(const StateType& snapshot) const
        {
            std::vector<StateListener<StateType>*> listeners_copy;
            {
                std::lock_guard lock(storage_->listeners_mutex);
                listeners_copy = storage_->listeners;
            }

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