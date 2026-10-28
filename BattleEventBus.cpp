#include "BattleEventBus.h"
#include <algorithm>
#include <utility>

namespace BattleEvent {

    Bus& Bus::Instance() {
        static Bus bus;
        return bus;
    }

    void Bus::AddHandler(std::unique_ptr<IHandler> handler) {
        if (!handler) return;
        handlers_.push_back(std::move(handler));
    }

    void Bus::RemoveHandlerByOwner(int ownerId) {
        if (ownerId == -1) return;
        if (depth_ > 0) {
            pendingRemoval_.push_back(ownerId);
            return;
        }
        std::erase_if(handlers_, [ownerId](const auto& h) {
            return h && h->OwnerId() == ownerId;
            });
    }

    void Bus::Clear() {
        handlers_.clear();
        pending_.clear();
        pendingRemoval_.clear();
        depth_ = 0;
    }

    std::size_t Bus::Size() const {
        return handlers_.size();
    }

    void Bus::Emit(IContext& ctx, const Event& e) {
        pending_.push_back(e);
        if (depth_ > 0) return;

        ++depth_;
        while (!pending_.empty()) {
            Event event = std::move(pending_.back());
            pending_.pop_back();

            for (auto& handler : handlers_) {
                if (handler) handler->OnEvent(ctx, event);
            }

            // removal requested while dispatching is applied after this event
            if (!pendingRemoval_.empty()) {
                for (int ownerId : pendingRemoval_) {
                    std::erase_if(handlers_, [ownerId](const auto& h) {
                        return h && h->OwnerId() == ownerId;
                        });
                }
                pendingRemoval_.clear();
            }
        }
        --depth_;
    }

}
