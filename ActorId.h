#pragma once
#include<numeric>
#include<climits>
#include<limits>
namespace Fight {
	struct ActorId {
		static constexpr int InvalidIndex = std::numeric_limits<int>::max();
		int index = InvalidIndex;
		int version = 0;
		bool operator==(const ActorId& o)const {
			return index == o.index && version == o.version;
		}
		bool operator!=(const ActorId& o)const {
			return index != o.index || version != o.version;
		}
		bool valid()const {
			return index != InvalidIndex;
		}
	};
}
