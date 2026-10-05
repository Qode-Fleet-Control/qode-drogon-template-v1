#include "RootCtrl.h"

void RootCtrl::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    // write your application logic here
    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_PLAIN);
    resp->setBody("Hello from the Drogon template!\n");
    callback(resp);
}
