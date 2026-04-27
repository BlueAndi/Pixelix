---
name: openapi-spec
description: 'Write and update OpenAPI 3.0 specification files from REST API code. Use when: documenting REST endpoints, creating API specs, generating swagger.yaml, updating API documentation from implementation.'
argument-hint: 'Describe the API endpoints to document or specify the code files containing REST handlers'
---

# OpenAPI 3.0 Specification Writer

## When to Use

Load this skill when asked to:
- Create an OpenAPI/Swagger specification from existing REST API code
- Document REST endpoints in OpenAPI 3.0 format
- Update an existing `openapi.yaml` or `swagger.yaml` file
- Generate API documentation that follows OpenAPI 3.0 standards
- Extract endpoint definitions, parameters, and responses from code

## Ground Rules

- Target specification: **OpenAPI 3.0.x** (not Swagger 2.0)
- Output format: YAML (preferred) or JSON
- Follow [OpenAPI 3.0 Specification](https://swagger.io/docs/specification/v3_0/about/)
- Include examples for complex schemas
- Document all response codes actually returned by the implementation

## Procedure

### 1. Analyze the REST API Code

#### Static Route Registration

Search for and identify:
- HTTP request handlers and route definitions (e.g., `server.on()`, route arrays)
- HTTP methods (GET, POST, PUT, PATCH, DELETE, OPTIONS)
- Path patterns and parameters (`:id`, `{id}`, wildcards)
- Query parameters from `request->getParam()` or equivalent
- Request body parsing (JSON, form data, multipart)
- Response codes and JSON structures
- Authentication/authorization requirements
- Error responses and status codes

#### Dynamic Route Registration

Some REST APIs use plugin or service architectures where endpoints are registered dynamically:

**Common patterns to search for:**
- `getTopics()` or `registerTopics()` methods in plugins/services
- Topic handler or topic registration services
- Topic constants (e.g., `TOPIC_CONFIG`, `TOPIC_STATUS`)
- Dynamic endpoint builders that construct paths from entity IDs and topic names

**Topic-based endpoint patterns:**
- Plugin endpoints: `/api/v1/display/uid/{uid}/{topic}` or `/display/alias/{alias}/{topic}`
- Service endpoints: `/api/v1/{entityId}/{topic}`
- System endpoints: `/api/v1/{topic}` (empty entityId)
- Indexed endpoints: `/api/v1/{entityId}/{index}/{topic}`

**How to find dynamic endpoints:**
1. Search for `getTopics()` implementations - these list available topics
2. Find `getTopic()` and `setTopic()` methods - these handle GET/POST requests
3. Locate topic registration code - shows how topics become REST endpoints
4. Check for `TopicHandlerService` or similar dynamic registration systems
5. Look for topic constants defined in header or source files

**Example (C++):**
```cpp
// Plugin defines topics
const char* TOPIC_CONFIG = "config";
const char* TOPIC_STATUS = "status";

void Plugin::getTopics(JsonArray& topics) const
{
    topics.add(TOPIC_CONFIG);  // Creates GET/POST /display/uid/{uid}/config
    topics.add(TOPIC_STATUS);  // Creates GET/POST /display/uid/{uid}/status
}

bool Plugin::getTopic(const String& topic, JsonObject& value) const
{
    if (topic.equals(TOPIC_CONFIG)) {
        // Handle GET request
    }
}

bool Plugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    if (topic.equals(TOPIC_CONFIG)) {
        // Handle POST request
    }
}
```

### 2. Discover All Endpoints

**Critical**: Don't assume you've found all endpoints after discovering static routes.

**Complete discovery workflow:**

1. **Static routes**: Find route registration arrays or explicit route handlers
2. **Plugin topics**: Search for classes implementing plugin interfaces
   - Look for `getTopics()` implementations
   - Each plugin may expose multiple topics as REST endpoints
3. **Service topics**: Search for service classes that register topics
   - Services often register multiple topics (e.g., files, upload, remove)
4. **Topic registration**: Find the topic handler or registration service
   - Understand how topics are converted to REST paths
   - Check for path prefix construction (e.g., base + entityId + topic)
5. **Validate completeness**: Cross-reference existing OpenAPI spec
   - Check for endpoints in old spec that might still exist
   - Verify each documented endpoint still exists in code
   - Add missing endpoints found in code

**Search patterns:**
```bash
# Find topic definitions
grep -r "TOPIC_" --include="*.cpp" --include="*.h"

# Find getTopics implementations
grep -r "getTopics" --include="*.cpp"

# Find topic handlers
grep -r "getTopic\|setTopic" --include="*.cpp"

# Find topic registration
grep -r "registerTopic" --include="*.cpp"
```

### 3. Build the OpenAPI Structure

Start with the [base template](./references/openapi-template.yaml):

```yaml
openapi: 3.0.3
info:
  title: [API Name]
  version: [Version from VERSION constant or git tag]
  description: [Brief API description]
  contact:
    name: [From LICENSE or README]
    email: [If available]

servers:
  - url: http://{host}/rest/api/v1
    description: REST API base path
    variables:
      host:
        default: localhost
        description: Device hostname or IP

paths:
  # Endpoints go here

components:
  schemas:
    # Reusable schemas
  responses:
    # Common responses
  securitySchemes:
    # Auth schemes
```

### 4. Document Each Endpoint

For each route/handler found:

```yaml
/path/{param}:
  get:
    summary: [One-line description from code comments]
    description: [Detailed behavior from docstrings/comments]
    operationId: [camelCase unique identifier]
    tags:
      - [Logical grouping]
    parameters:
      - name: param
        in: path
        required: true
        schema:
          type: string
        description: [From parameter docs]
      - name: query
        in: query
        required: false
        schema:
          type: integer
        description: [From code inspection]
    responses:
      '200':
        description: Success
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/SuccessResponse'
            example:
              status: "ok"
              data: {}
      '400':
        $ref: '#/components/responses/BadRequest'
      '404':
        $ref: '#/components/responses/NotFound'
```

### 5. Extract Response Schemas

From JSON response building code, create reusable schemas:

```yaml
components:
  schemas:
    SuccessResponse:
      type: object
      required:
        - status
        - data
      properties:
        status:
          type: string
          enum: [ok]
        data:
          type: object
          description: Endpoint-specific response data
    
    ErrorResponse:
      type: object
      required:
        - status
        - msg
      properties:
        status:
          type: string
          enum: [error]
        msg:
          type: string
          description: Human-readable error message
```

### 6. Document Authentication

If authentication is present:

```yaml
components:
  securitySchemes:
    basicAuth:
      type: http
      scheme: basic
      description: HTTP Basic Authentication

# Then apply to protected endpoints:
paths:
  /protected:
    get:
      security:
        - basicAuth: []
```

### 7. Validate and Refine

- Check that all paths start with `/`
- Verify all `$ref` references exist
- Ensure required fields are present
- Add examples for complex request/response bodies
- Group related endpoints with `tags`
- Document error responses consistently

## Output Format

Save the specification as:
- `docs/openapi.yaml` or `docs/swagger.yaml` (YAML preferred)
- `docs/api-spec.yaml` (alternative naming)

Include a comment header:
```yaml
# OpenAPI 3.0.3 Specification
# Generated from: [source files]
# Last updated: [date]
# See: https://swagger.io/docs/specification/v3_0/
```

## Common Patterns

### REST API with CRUD Operations

```yaml
/items:
  get:
    summary: List all items
    responses:
      '200':
        description: Array of items
  post:
    summary: Create new item
    requestBody:
      required: true
      content:
        application/json:
          schema:
            $ref: '#/components/schemas/ItemCreate'
    responses:
      '201':
        description: Item created

/items/{id}:
  get:
    summary: Get single item
  put:
    summary: Update item
  delete:
    summary: Delete item
```

### Query Parameters

```yaml
parameters:
  - name: limit
    in: query
    schema:
      type: integer
      minimum: 1
      maximum: 100
      default: 20
  - name: offset
    in: query
    schema:
      type: integer
      minimum: 0
      default: 0
```

### File Upload

```yaml
requestBody:
  content:
    multipart/form-data:
      schema:
        type: object
        properties:
          file:
            type: string
            format: binary
          path:
            type: string
```

## Common Pitfalls & Lessons Learned

### File Paths vs Numeric IDs

**Issue**: APIs may use numeric IDs internally instead of file paths.

**Example:**
```yaml
# WRONG - assumes API uses file paths
parameters:
  - name: iconPath
    schema:
      type: string
    example: "/images/icon.bmp"

# CORRECT - API uses numeric file IDs
parameters:
  - name: iconFileId
    schema:
      type: number
    example: 1234
```

**How to identify**: Check the actual implementation - look for `FileId` types, ID resolution methods like `getFileFullPathById()`, or file manager services that map IDs to paths.

### Missing Dynamic Endpoints

**Issue**: Forgetting to document endpoints that are registered dynamically via plugin/service systems.

**Solution**:
1. Don't rely only on static route definitions
2. Search for `getTopics()`, `registerTopics()`, or similar methods
3. Check for topic registration services that create REST endpoints
4. Look for topic handler implementations
5. Verify each plugin/service that registers topics

**Example**: In Pixelix, sensor endpoints (`/sensors/{index}/{channelName}`) are created dynamically by `SensorDataProvider` registering topics via `TopicHandlerService`, not found in static route arrays.

### Path Pattern Consistency

**Issue**: API paths may have multiple formats depending on registration method.

**Check for**:
- Service endpoints: `/{serviceId}/{topic}`
- Plugin endpoints by UID: `/display/uid/{uid}/{topic}`
- Plugin endpoints by alias: `/display/alias/{alias}/{topic}`
- Indexed endpoints: `/{entityId}/{index}/{topic}`
- System endpoints: `/{topic}` (no prefix)

### Parameter Types from Code

**Issue**: Documentation doesn't match actual parameter types used in code.

**Verify**:
- Check actual JSON key names in `getTopic()`/`setTopic()` implementations
- Confirm data types (string, number, boolean, array, object)
- Note optional vs required parameters
- Check for parameter validation rules (min/max, enums)

### Configuration vs Command Topics

**Issue**: Some topics serve dual purposes.

**Pattern**:
- **Config topics**: GET returns current config, POST updates and persists config
- **Command topics**: GET returns status, POST executes actions with `action` parameter
- **Status topics**: GET only, returns current state

**Example:**
```yaml
# Command topic - action-based
/display/uid/{uid}/playCtrl:
  post:
    parameters:
      - name: action
        schema:
          type: string
          enum: [next, previous, pause, continue]
```

## Tips

- **Use `$ref` for reusability**: Common responses and schemas should be defined once in `components`
- **Include examples**: Especially for complex nested objects
- **Document all response codes**: Even error cases (400, 401, 403, 404, 500)
- **Add operation IDs**: Unique, descriptive `operationId` for code generation tools
- **Group with tags**: Logical grouping improves generated documentation (e.g., by plugin name or service name)
- **Version properly**: Use `info.version` matching your API versioning scheme
- **Add curl examples**: Include practical curl command examples in endpoint descriptions to show authentication and parameter usage
- **Consistency matters**: Keep response schemas, error formats, and authentication patterns consistent across all endpoints
- **Trace the implementation**: Don't guess parameter types or structures - read the actual code that builds JSON responses

## Validation

After generating the spec, validate it using:
- [Swagger Editor](https://editor.swagger.io/) - paste YAML to check for errors
- `swagger-cli validate openapi.yaml` - CLI validation
- VS Code OpenAPI extensions for real-time validation

## References

- [OpenAPI 3.0 Specification](https://swagger.io/docs/specification/v3_0/about/)
- [OpenAPI Data Types](https://swagger.io/docs/specification/data-models/data-types/)
- [Authentication](https://swagger.io/docs/specification/authentication/)
- [Example template](./references/openapi-template.yaml)
