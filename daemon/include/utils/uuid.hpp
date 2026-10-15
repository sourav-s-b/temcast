#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace utils {

using uuid = boost::uuids::uuid;

inline uuid geneate_id() { return boost::uuids::random_generator()(); }

} // namespace utils
