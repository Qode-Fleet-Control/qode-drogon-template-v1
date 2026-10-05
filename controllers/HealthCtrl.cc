#include "HealthCtrl.h"

void HealthCtrl::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    // write your application logic here
    Json::Value body;
    body["status"] = "ok";
    callback(HttpResponse::newHttpJsonResponse(body));
}
