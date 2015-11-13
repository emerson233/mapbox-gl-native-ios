#include "storage.hpp"

#include <uv.h>

#include <mbgl/storage/default_file_source.hpp>
#include <mbgl/storage/network_status.hpp>
#include <mbgl/util/run_loop.hpp>

#include <cmath>

TEST_F(Storage, HTTPCancel) {
    SCOPED_TEST(HTTPCancel)

    using namespace mbgl;

    DefaultFileSource fs(nullptr);
    util::RunLoop loop(uv_default_loop());

    auto req =
        fs.request({ Resource::Unknown, "http://127.0.0.1:3000/test" },
                   [&](Response) { ADD_FAILURE() << "Callback should not be called"; });

    fs.cancel(req);
    HTTPCancel.finish();

    uv_run(uv_default_loop(), UV_RUN_ONCE);
}

TEST_F(Storage, HTTPCancelMultiple) {
    SCOPED_TEST(HTTPCancelMultiple)

    using namespace mbgl;

    DefaultFileSource fs(nullptr);
    util::RunLoop loop(uv_default_loop());

    const Resource resource { Resource::Unknown, "http://127.0.0.1:3000/test" };

    auto req2 = fs.request(resource, [&](Response) {
        ADD_FAILURE() << "Callback should not be called";
    });
    Request* req = fs.request(resource, [&](Response res) {
        fs.cancel(req);
        EXPECT_EQ(nullptr, res.error);
        EXPECT_EQ(false, res.stale);
        ASSERT_TRUE(res.data.get());
        EXPECT_EQ("Hello World!", *res.data);
        EXPECT_EQ(0, res.expires);
        EXPECT_EQ(0, res.modified);
        EXPECT_EQ("", res.etag);
        loop.stop();
        HTTPCancelMultiple.finish();
    });
    fs.cancel(req2);

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
