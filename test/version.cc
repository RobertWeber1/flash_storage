#include "catch.h"
#include <flash_storage/version.h>

using namespace flash_storage;

TEST_CASE("check version")
{
	REQUIRE(Version::project_name == std::string("flash_storage"));
	REQUIRE(Version::commit_hash != std::string(""));
	REQUIRE(Version::git_tag != std::string(""));
}
