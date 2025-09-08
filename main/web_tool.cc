#include "web_tool.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "mcp_server.h" // We need this to register the tool

#define TAG "WebTool"

// Helper function to handle the HTTP event data
esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    static std::string* response_buffer = nullptr;
    if (evt->event_id == HTTP_EVENT_ON_DATA) {
        if (response_buffer) {
            response_buffer->append((char*)evt->data, evt->data_len);
        }
    } else if (evt->event_id == HTTP_EVENT_ON_FINISH) {
        response_buffer = nullptr;
    } else if (evt->event_id == HTTP_EVENT_ON_CONNECTED) {
        response_buffer = (std::string*)evt->user_data;
        response_buffer->clear();
    }
    return ESP_OK;
}

// The core logic function
std::string perform_web_search(const std::string& query) {
    std::string response_data;
    std::string url = "https://httpbin.org/get?query=" + query;

    esp_http_client_config_t config = {
        .url = url.c_str(),
        .event_handler = http_event_handler,
        .user_data = &response_data, // Pass our response string to the handler
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET request successful, response: %s", response_data.c_str());
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
        response_data = "Error: Failed to fetch web data.";
    }

    esp_http_client_cleanup(client);
    return response_data;
}

// The function to register our tool
void register_web_tool() {
    auto& mcp_server = McpServer::GetInstance();

    mcp_server.AddTool(
        "self.get_web_info",                                 // The tool's unique name
        "Fetches information from the web based on a query", // A description for the AI
        PropertyList({                                       // Define the input parameters
            Property("query", kPropertyTypeString, "The search query")
        }),
        // This is the C++ lambda function that gets executed when the tool is called
        [](const PropertyList& properties) -> ReturnValue {
            std::string query = properties["query"].value<std::string>();
            ESP_LOGI(TAG, "MCP tool 'get_web_info' called with query: %s", query.c_str());

            // Call our core logic function
            std::string result = perform_web_search(query);

            // Return the result to the server
            return result;
        }
    );
    ESP_LOGI(TAG, "Web search tool registered successfully.");
}