# API Documentation

## Groq API Integration

M5GroqBot uses two Groq API endpoints:

### 1. Whisper API (Speech-to-Text)

**Endpoint:** `https://api.groq.com/openai/v1/audio/transcriptions`

**Model:** `whisper-large-v3-turbo`

**Audio Format:**
- Sample Rate: 16kHz
- Channels: Mono (1)
- Bit Depth: 16-bit
- Format: WAV
- Duration: 2 seconds

**Request:**
```http
POST /openai/v1/audio/transcriptions HTTP/1.1
Host: api.groq.com
Authorization: Bearer YOUR_API_KEY
Content-Type: multipart/form-data; boundary=M5Boundary

--M5Boundary
Content-Disposition: form-data; name="model"

whisper-large-v3-turbo
--M5Boundary
Content-Disposition: form-data; name="file"; filename="audio.wav"
Content-Type: audio/wav

[WAV FILE DATA]
--M5Boundary--
```

**Response:**
```json
{
  "text": "What is the weather like today?"
}
```

### 2. Chat Completions API (AI Response)

**Endpoint:** `https://api.groq.com/openai/v1/chat/completions`

**Model:** `llama-3.3-70b-versatile`

**Request:**
```http
POST /openai/v1/chat/completions HTTP/1.1
Host: api.groq.com
Authorization: Bearer YOUR_API_KEY
Content-Type: application/json

{
  "model": "llama-3.3-70b-versatile",
  "messages": [
    {
      "role": "system",
      "content": "You are a helpful assistant. Keep responses very concise (2-3 sentences max)."
    },
    {
      "role": "user",
      "content": "What is the weather like today?"
    }
  ],
  "temperature": 0.7,
  "max_tokens": 300
}
```

**Response:**
```json
{
  "id": "chatcmpl-123",
  "object": "chat.completion",
  "created": 1699999999,
  "model": "llama-3.3-70b-versatile",
  "choices": [
    {
      "index": 0,
      "message": {
        "role": "assistant",
        "content": "I don't have access to real-time weather data. You can check weather.com or your local weather service for current conditions in your area."
      },
      "finish_reason": "stop"
    }
  ]
}
```

## Configuration

### System Prompt
Default system prompt limits response length:
```
"You are a helpful assistant. Keep responses very concise (2-3 sentences max)."
```

### Chat Parameters
- **Temperature:** 0.7 (balanced creativity/accuracy)
- **Max Tokens:** 300 (limits response length)
- **Top-p:** Not specified (default)

### Conversation History
- Stores last 5 exchanges (10 messages)
- Saved to device preferences
- Maintains context across restarts
- Cleared on manual reset

## Rate Limits

Groq Free Tier (as of 2025):
- Requests per minute: Varies by model
- Requests per day: Limited
- Check current limits: [console.groq.com/docs/rate-limits](https://console.groq.com/docs/rate-limits)

**Rate Limit Handling:**
Device does not implement retry logic. If rate limited:
1. Wait 60 seconds
2. Try again
3. Consider upgrading Groq plan

## Error Handling

### Whisper API Errors

| Error | Cause | Device Response |
|-------|-------|-----------------|
| 401 Unauthorized | Invalid API key | "Error: No API key" |
| 400 Bad Request | Invalid audio format | "Error: Parse failed" |
| 413 Payload Too Large | Audio file too big | N/A (2s limit enforced) |
| 429 Too Many Requests | Rate limited | "Error: [HTTP 429]" |
| 500 Server Error | Groq API issue | "Error: [HTTP 500]" |

### Chat API Errors

| Error | Cause | Device Response |
|-------|-------|-----------------|
| 401 Unauthorized | Invalid API key | "Error: No API key" |
| 400 Bad Request | Invalid request format | "Error: Parse failed" |
| 429 Too Many Requests | Rate limited | "Error: [HTTP 429]" |
| 500 Server Error | Groq API issue | "Error: [HTTP 500]" |

## Security Considerations

### API Key Storage
- Stored in ESP32 NVS (Non-Volatile Storage)
- Namespace: "groqbot"
- Not encrypted
- ⚠️ **Do not share device backups publicly**

### Network Security
- Uses WiFiClientSecure for HTTPS
- Certificate validation disabled (`.setInsecure()`)
- Audio transmitted over secure connection
- API key in Authorization header

### Privacy
- Audio sent to Groq servers
- Transcriptions and responses stored locally
- No data sent to third parties
- Clear history by resetting device

## API Key Setup

### Get Free API Key
1. Go to [console.groq.com](https://console.groq.com)
2. Sign up for free account
3. Navigate to API Keys section
4. Click "Create API Key"
5. Copy key (starts with `gsk_...`)
6. Enter during device setup

### Key Permissions
API key needs:
- Whisper API access
- Chat API access
- No admin permissions required

## Response Formatting

### Text Processing
Device applies basic formatting:
- Word wrapping at word boundaries
- Line breaks preserved
- Max display length: ~200 chars per screen
- Scroll for longer responses

### Special Characters
Most Unicode supported, but limited by font:
- ASCII fully supported
- Emojis: Limited support
- Extended Unicode: May display as `?`

## Memory Management

### Conversation Context
```cpp
// Stored in preferences
String chatHistory = "";  // JSON array of messages
int messageCount = 0;     // Number of exchanges

// Format:
[
  {"role": "user", "content": "Question 1"},
  {"role": "assistant", "content": "Answer 1"},
  {"role": "user", "content": "Question 2"},
  {"role": "assistant", "content": "Answer 2"}
]
```

### Memory Limits
- DynamicJsonDocument: 8KB for responses
- Chat history: 4KB
- Audio buffer: 64KB (PSRAM)
- WAV buffer: 64KB + 44 bytes (PSRAM)

## Extending the API

### Custom System Prompt
Edit in `main.cpp`:
```cpp
JsonObject system = messages.createNestedObject();
system["role"] = "system";
system["content"] = "Your custom system prompt here";
```

### Different Models
Change model constants:
```cpp
#define GROQ_MODEL "mixtral-8x7b-32768"  // Alternative model
#define GROQ_WHISPER_MODEL "whisper-large-v3"  // Different Whisper
```

### Adjust Response Length
```cpp
doc["max_tokens"] = 500;  // Longer responses
doc["temperature"] = 0.5;  // More conservative
```

## Testing

### Test Whisper API
```bash
curl https://api.groq.com/openai/v1/audio/transcriptions \
  -H "Authorization: Bearer $GROQ_API_KEY" \
  -F model="whisper-large-v3-turbo" \
  -F file="@test.wav"
```

### Test Chat API
```bash
curl https://api.groq.com/openai/v1/chat/completions \
  -H "Authorization: Bearer $GROQ_API_KEY" \
  -H "Content-Type: application/json" \
  -d '{
    "model": "llama-3.3-70b-versatile",
    "messages": [{"role": "user", "content": "Hello"}]
  }'
```

## References

- [Groq API Documentation](https://console.groq.com/docs)
- [Groq Models](https://console.groq.com/docs/models)
- [OpenAI API Compatibility](https://console.groq.com/docs/openai)
- [Rate Limits](https://console.groq.com/docs/rate-limits)

---

For questions or issues with the Groq API, contact [Groq Support](https://console.groq.com).
