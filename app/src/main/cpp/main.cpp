
#include <android/log.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <cmath>
#include <vector>

#define LOG_TAG "NDKGame"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

struct Vertex {
    float x, y, z;
    float r, g, b, a;
};

struct Player {
    float x, y, z;
    float size;
    float color[4];
    float speed;
};

struct Ball {
    float x, y, z;
    float radius;
    float color[4];
    float velocityX, velocityY;
};

struct GameState {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    bool initialized;
    int width, height;
    float aspectRatio;
    
    GLuint program;
    GLuint vertexBuffer;
    
    Player player1;
    Player player2;
    Ball ball;
    
    bool touchActive;
    float touchX, touchY;
    
    float fieldWidth, fieldHeight;
    float boundaryMargin;
    
    float projectionMatrix[16];
};

static const char vertexShaderSource[] = 
    "uniform mat4 uProjectionMatrix;\n"
    "attribute vec4 aPosition;\n"
    "attribute vec4 aColor;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "   gl_Position = uProjectionMatrix * aPosition;\n"
    "   vColor = aColor;\n"
    "}\n";

static const char fragmentShaderSource[] = 
    "precision mediump float;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "   gl_FragColor = vColor;\n"
    "}\n";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        LOGE("Shader compilation failed: %s", infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    if (!vertexShader || !fragmentShader) {
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        LOGE("Program linking failed: %s", infoLog);
        glDeleteProgram(program);
        program = 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

void createCubeVertices(std::vector<Vertex>& vertices, float centerX, float centerY, float centerZ, 
                        float size, const float color[4]) {
    float halfSize = size / 2.0f;
    
    // Front face
    vertices.push_back({centerX - halfSize, centerY - halfSize, centerZ + halfSize, color[0], color[1], color[2], color[3]});
    vertices.push_back({centerX + halfSize, centerY - halfSize, centerZ + halfSize, color[0], color[1], color[2], color[3]});
    vertices.push_back({centerX - halfSize, centerY + halfSize, centerZ + halfSize, color[0], color[1], color[2], color[3]});
    vertices.push_back({centerX + halfSize, centerY + halfSize, centerZ + halfSize, color[0], color[1], color[2], color[3]});
    
    // Back face
    vertices.push_back({centerX - halfSize, centerY - halfSize, centerZ - halfSize, color[0], color[1], color[2], color[3]});
    vertices.push_back({centerX + halfSize, centerY - halfSize, centerZ - halfSize, color[0], color[1], color[2], color[3]});
    vertices.push_back({centerX - halfSize, centerY + halfSize, centerZ - halfSize, color[0], color[1], color[2], color[3]});
    vertices.push_back({centerX + halfSize, centerY + halfSize, centerZ - halfSize, color[0], color[1], color[2], color[3]});
}

void createSphereVertices(std::vector<Vertex>& vertices, float centerX, float centerY, float centerZ,
                         float radius, const float color[4], int segments = 16) {
    for (int i = 0; i <= segments; i++) {
        float lat0 = M_PI * (-0.5f + (float)(i - 1) / segments);
        float z0 = radius * sin(lat0);
        float r0 = radius * cos(lat0);
        
        float lat1 = M_PI * (-0.5f + (float)i / segments);
        float z1 = radius * sin(lat1);
        float r1 = radius * cos(lat1);
        
        for (int j = 0; j <= segments; j++) {
            float lng = 2 * M_PI * (float)(j - 1) / segments;
            float x0 = cos(lng) * r0;
            float y0 = sin(lng) * r0;
            
            lng = 2 * M_PI * (float)j / segments;
            float x1 = cos(lng) * r0;
            float y1 = sin(lng) * r0;
            float x2 = cos(lng) * r1;
            float y2 = sin(lng) * r1;
            
            vertices.push_back({centerX + x0, centerY + y0, centerZ + z0, color[0], color[1], color[2], color[3]});
            vertices.push_back({centerX + x1, centerY + y1, centerZ + z0, color[0], color[1], color[2], color[3]});
            vertices.push_back({centerX + x2, centerY + y2, centerZ + z1, color[0], color[1], color[2], color[3]});
        }
    }
}

void createFieldVertices(std::vector<Vertex>& vertices, float width, float height, float margin) {
    float halfW = width / 2.0f;
    float halfH = height / 2.0f;
    float z = -0.5f;
    
    // Field surface (green)
    vertices.push_back({-halfW + margin, -halfH + margin, z, 0.0f, 0.5f, 0.0f, 1.0f});
    vertices.push_back({halfW - margin, -halfH + margin, z, 0.0f, 0.5f, 0.0f, 1.0f});
    vertices.push_back({-halfW + margin, halfH - margin, z, 0.0f, 0.5f, 0.0f, 1.0f});
    vertices.push_back({halfW - margin, halfH - margin, z, 0.0f, 0.5f, 0.0f, 1.0f});
    
    // Field boundaries (white)
    float boundaryZ = z + 0.1f;
    // Top boundary
    vertices.push_back({-halfW, halfH, boundaryZ, 1.0f, 1.0f, 1.0f, 1.0f});
    vertices.push_back({halfW, halfH, boundaryZ, 1.0f, 1.0f, 1.0f, 1.0f});
    // Bottom boundary
    vertices.push_back({-halfW, -halfH, boundaryZ, 1.0f, 1.0f, 1.0f, 1.0f});
    vertices.push_back({halfW, -halfH, boundaryZ, 1.0f, 1.0f, 1.0f, 1.0f});
    // Left boundary
    vertices.push_back({-halfW, -halfH, boundaryZ, 1.0f, 1.0f, 1.0f, 1.0f});
    vertices.push_back({-halfW, halfH, boundaryZ, 1.0f, 1.0f, 1.0f, 1.0f});
    // Right boundary
    vertices.push_back({halfW, -halfH, boundaryZ, 1.0f, 1.0f, 1.0f, 1.0f});
    vertices.push_back({halfW, halfH, boundaryZ, 1.0f, 1.0f, 1.0f, 1.0f});
}

void updateProjectionMatrix(GameState* state) {
    float left = -state->fieldWidth / 2.0f;
    float right = state->fieldWidth / 2.0f;
    float bottom = -state->fieldHeight / 2.0f;
    float top = state->fieldHeight / 2.0f;
    float near = -10.0f;
    float far = 10.0f;
    
    float* m = state->projectionMatrix;
    
    m[0] = 2.0f / (right - left);
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    
    m[4] = 0.0f;
    m[5] = 2.0f / (top - bottom);
    m[6] = 0.0f;
    m[7] = 0.0f;
    
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = -2.0f / (far - near);
    m[11] = 0.0f;
    
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
}

void initGame(GameState* state) {
    state->fieldWidth = 10.0f;
    state->fieldHeight = 15.0f;
    state->boundaryMargin = 0.2f;
    
    // Initialize players
    state->player1 = {0.0f, -state->fieldHeight/2 + 2.0f, 0.0f, 0.5f, {1.0f, 0.0f, 0.0f, 1.0f}, 0.1f};
    state->player2 = {0.0f, state->fieldHeight/2 - 2.0f, 0.0f, 0.5f, {0.0f, 0.0f, 1.0f, 1.0f}, 0.1f};
    
    // Initialize ball
    state->ball = {0.0f, 0.0f, 0.0f, 0.3f, {1.0f, 1.0f, 1.0f, 1.0f}, 0.05f, 0.05f};
    
    state->touchActive = false;
    
    updateProjectionMatrix(state);
    
    LOGI("Game initialized");
}

void updateGame(GameState* state) {
    // Move ball
    state->ball.x += state->ball.velocityX;
    state->ball.y += state->ball.velocityY;
    
    // Ball boundary collision
    float halfW = state->fieldWidth / 2.0f - state->boundaryMargin;
    float halfH = state->fieldHeight / 2.0f - state->boundaryMargin;
    
    if (state->ball.x - state->ball.radius < -halfW || 
        state->ball.x + state->ball.radius > halfW) {
        state->ball.velocityX = -state->ball.velocityX;
    }
    
    if (state->ball.y - state->ball.radius < -halfH || 
        state->ball.y + state->ball.radius > halfH) {
        state->ball.velocityY = -state->ball.velocityY;
    }
    
    // Move player based on touch
    if (state->touchActive) {
        // Convert touch coordinates to game coordinates
        float gameX = (state->touchX / state->width - 0.5f) * state->fieldWidth;
        float gameY = (0.5f - state->touchY / state->height) * state->fieldHeight;
        
        // Determine nearest player
        float dist1 = sqrt(pow(gameX - state->player1.x, 2) + pow(gameY - state->player1.y, 2));
        float dist2 = sqrt(pow(gameX - state->player2.x, 2) + pow(gameY - state->player2.y, 2));
        
        Player* targetPlayer = (dist1 < dist2) ? &state->player1 : &state->player2;
        
        // Calculate direction to touch point
        float dx = gameX - targetPlayer->x;
        float dy = gameY - targetPlayer->y;
        float distance = sqrt(dx * dx + dy * dy);
        
        if (distance > 0.1f) {
            dx /= distance;
            dy /= distance;
            
            // Move player
            targetPlayer->x += dx * targetPlayer->speed;
            targetPlayer->y += dy * targetPlayer->speed;
            
            // Keep player within boundaries
            float playerHalfSize = targetPlayer->size / 2.0f;
            targetPlayer->x = fmax(-halfW + playerHalfSize, fmin(halfW - playerHalfSize, targetPlayer->x));
            targetPlayer->y = fmax(-halfH + playerHalfSize, fmin(halfH - playerHalfSize, targetPlayer->y));
        }
    }
}

void renderGame(GameState* state) {
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glUseProgram(state->program);
    
    GLint projectionLoc = glGetUniformLocation(state->program, "uProjectionMatrix");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, state->projectionMatrix);
    
    GLint positionLoc = glGetAttribLocation(state->program, "aPosition");
    GLint colorLoc = glGetAttribLocation(state->program, "aColor");
    
    glEnableVertexAttribArray(positionLoc);
    glEnableVertexAttribArray(colorLoc);
    
    // Render field
    std::vector<Vertex> vertices;
    createFieldVertices(vertices, state->fieldWidth, state->fieldHeight, state->boundaryMargin);
    
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                         &vertices[0].x);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                         &vertices[0].r);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  // Field surface
    
    glDrawArrays(GL_LINES, 4, 8);  // Field boundaries
    
    // Render players
    vertices.clear();
    createCubeVertices(vertices, state->player1.x, state->player1.y, state->player1.z, 
                      state->player1.size, state->player1.color);
    createCubeVertices(vertices, state->player2.x, state->player2.y, state->player2.z, 
                      state->player2.size, state->player2.color);
    
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                         &vertices[0].x);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                         &vertices[0].r);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);  // Player 1
    glDrawArrays(GL_TRIANGLE_STRIP, 8, 8);  // Player 2
    
    // Render ball
    vertices.clear();
    createSphereVertices(vertices, state->ball.x, state->ball.y, state->ball.z, 
                        state->ball.radius, state->ball.color);
    
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                         &vertices[0].x);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                         &vertices[0].r);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    
    eglSwapBuffers(state->display, state->surface);
}

void shutdownGame(GameState* state) {
    if (state->program) {
        glDeleteProgram(state->program);
        state->program = 0;
    }
    state->initialized = false;
    LOGI("Game shutdown");
}

void handleTouchEvent(GameState* state, AInputEvent* event) {
    int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
    
    if (action == AMOTION_EVENT_ACTION_DOWN || 
        action == AMOTION_EVENT_ACTION_MOVE) {
        state->touchActive = true;
        state->touchX = AMotionEvent_getX(event, 0);
        state->touchY = AMotionEvent_getY(event, 0);
    } else if (action == AMOTION_EVENT_ACTION_UP) {
        state->touchActive = false;
    }
}

void handleAppCommand(android_app* app, int32_t cmd) {
    GameState* state = (GameState*)app->userData;
    
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (!state->initialized) {
                const EGLint attribs[] = {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                    EGL_BLUE_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_RED_SIZE, 8,
                    EGL_DEPTH_SIZE, 16,
                    EGL_NONE
                };
                
                EGLint contextAttribs[] = {
                    EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL_NONE
                };
                
                state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
                eglInitialize(state->display, 0, 0);
                
                EGLint numConfigs;
                EGLConfig config;
                eglChooseConfig(state->display, attribs, &config, 1, &numConfigs);
                
                state->surface = eglCreateWindowSurface(state->display, config, app->window, nullptr);
                state->context = eglCreateContext(state->display, config, nullptr, contextAttribs);
                
                eglMakeCurrent(state->display, state->surface, state->surface, state->context);
                
                state->program = createProgram();
                if (!state->program) {
                    LOGE("Failed to create shader program");
                    return;
                }
                
                initGame(state);
                state->initialized = true;
            }
            break;
            
        case APP_CMD_TERM_WINDOW:
            if (state->initialized) {
                shutdownGame(state);
                eglMakeCurrent(state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                if (state->context != EGL_NO_CONTEXT) {
                    eglDestroyContext(state->display, state->context);
                }
                if (state->surface != EGL_NO_SURFACE) {
                    eglDestroySurface(state->display, state->surface);
                }
                eglTerminate(state->display);
            }
            state->initialized = false;
            break;
            
        case APP_CMD_GAINED_FOCUS:
            break;
            
        case APP_CMD_LOST_FOCUS:
            break;
            
        case APP_CMD_WINDOW_RESIZED:
            if (state->initialized) {
                state->width = ANativeWindow_getWidth(app->window);
                state->height = ANativeWindow_getHeight(app->window);
                glViewport(0, 0, state->width, state->height);
                updateProjectionMatrix(state);
            }
            break;
    }
}

int32_t handleInputEvent(android_app* app, AInputEvent* event) {
    GameState* state = (GameState*)app->userData;
    
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        handleTouchEvent(state, event);
        return 1;
    }
    return 0;
}

void android_main(android_app* app) {
    app_dummy();
    
    GameState state = {};
    app->userData = &state;
    app->onAppCmd = handleAppCommand;
    app->onInputEvent = handleInputEvent;
    
    state.initialized = false;
    
    while (true) {
        int ident;
        int events;
        android_poll_source* source;
        
        while ((ident = ALooper_pollAll(0, nullptr, &events, (void**)&source)) >= 0) {
            if (source) {
                source->process(app, source);
            }
            
            if (app->destroyRequested) {
                if (state.initialized) {
                    shutdownGame(&state);
                }
                return;
            }
        }
        
        if (state.initialized) {
            updateGame(&state);
            renderGame(&state);
        }
    }
}
