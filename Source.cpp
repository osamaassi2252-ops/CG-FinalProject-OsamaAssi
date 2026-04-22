//==============================================================
// THE STOLEN DREAMS CASTLE - A Creative 3D Adventure
// جامعةعة: لعبة ثلاثية الأبعاد إبداعية مع تأثيرات بصرية مبهرة
//==============================================================
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <ctime>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;

// ==================== الثوابت ====================
const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;
const float PLAYER_SPEED = 5.0f;
const float CAM_DIST = 8.0f;
const float SENSITIVITY = 0.002f;
const float PLAYER_RADIUS = 0.4f;
const float DREAM_RADIUS = 0.5f;
const float EYE_RADIUS = 0.45f;
const float PORTAL_RADIUS = 1.0f;

enum GameState { PLAYING, WIN, LOSE };
enum PortalState { INACTIVE, ACTIVATING, ACTIVE };

// ==================== هيكل Vertex ====================
struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 tex;
};

// ==================== توليد المكعب ====================
vector<Vertex> genCubeVertices() {
    vector<Vertex> v;
    // Front
    v.push_back({ {-0.5f,-0.5f, 0.5f},{0,0,1},{0,0} });
    v.push_back({ { 0.5f,-0.5f, 0.5f},{0,0,1},{1,0} });
    v.push_back({ { 0.5f, 0.5f, 0.5f},{0,0,1},{1,1} });
    v.push_back({ {-0.5f, 0.5f, 0.5f},{0,0,1},{0,1} });
    // Back
    v.push_back({ {-0.5f,-0.5f,-0.5f},{0,0,-1},{0,0} });
    v.push_back({ { 0.5f,-0.5f,-0.5f},{0,0,-1},{1,0} });
    v.push_back({ { 0.5f, 0.5f,-0.5f},{0,0,-1},{1,1} });
    v.push_back({ {-0.5f, 0.5f,-0.5f},{0,0,-1},{0,1} });
    // Right
    v.push_back({ { 0.5f,-0.5f, 0.5f},{1,0,0},{0,0} });
    v.push_back({ { 0.5f,-0.5f,-0.5f},{1,0,0},{1,0} });
    v.push_back({ { 0.5f, 0.5f,-0.5f},{1,0,0},{1,1} });
    v.push_back({ { 0.5f, 0.5f, 0.5f},{1,0,0},{0,1} });
    // Left
    v.push_back({ {-0.5f,-0.5f, 0.5f},{-1,0,0},{0,0} });
    v.push_back({ {-0.5f,-0.5f,-0.5f},{-1,0,0},{1,0} });
    v.push_back({ {-0.5f, 0.5f,-0.5f},{-1,0,0},{1,1} });
    v.push_back({ {-0.5f, 0.5f, 0.5f},{-1,0,0},{0,1} });
    // Top
    v.push_back({ {-0.5f, 0.5f, 0.5f},{0,1,0},{0,0} });
    v.push_back({ { 0.5f, 0.5f, 0.5f},{0,1,0},{1,0} });
    v.push_back({ { 0.5f, 0.5f,-0.5f},{0,1,0},{1,1} });
    v.push_back({ {-0.5f, 0.5f,-0.5f},{0,1,0},{0,1} });
    // Bottom
    v.push_back({ {-0.5f,-0.5f, 0.5f},{0,-1,0},{0,0} });
    v.push_back({ { 0.5f,-0.5f, 0.5f},{0,-1,0},{1,0} });
    v.push_back({ { 0.5f,-0.5f,-0.5f},{0,-1,0},{1,1} });
    v.push_back({ {-0.5f,-0.5f,-0.5f},{0,-1,0},{0,1} });
    return v;
}

vector<unsigned int> genCubeIndices() {
    vector<unsigned int> ind;
    for (int i = 0; i < 6; i++) {
        int b = i * 4;
        ind.push_back(b); ind.push_back(b + 1); ind.push_back(b + 2);
        ind.push_back(b); ind.push_back(b + 2); ind.push_back(b + 3);
    }
    return ind;
}

// ==================== توليد الكرة (للأحلام والعيون) ====================
vector<Vertex> genSphereVertices(float r, int sec = 32, int stacks = 32) {
    vector<Vertex> v;
    for (int i = 0; i <= stacks; i++) {
        float phi = 3.14159265f * i / stacks;
        for (int j = 0; j <= sec; j++) {
            float theta = 2 * 3.14159265f * j / sec;
            float x = r * sin(phi) * cos(theta);
            float y = r * cos(phi);
            float z = r * sin(phi) * sin(theta);
            glm::vec3 norm = glm::normalize(glm::vec3(x, y, z));
            glm::vec2 tex = { (float)j / sec, (float)i / stacks };
            v.push_back({ {x,y,z}, norm, tex });
        }
    }
    return v;
}

vector<unsigned int> genSphereIndices(int sec, int stacks) {
    vector<unsigned int> ind;
    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < sec; j++) {
            int a = i * (sec + 1) + j;
            int b = a + sec + 1;
            ind.push_back(a); ind.push_back(b); ind.push_back(a + 1);
            ind.push_back(b); ind.push_back(b + 1); ind.push_back(a + 1);
        }
    }
    return ind;
}

// ==================== توليد الأسطوانة (للأبراج) ====================
vector<Vertex> genCylinderVertices(float r, float h, int segments = 24) {
    vector<Vertex> v;
    // Body
    for (int i = 0; i <= segments; i++) {
        float angle = 2 * 3.14159265f * i / segments;
        float x = r * cos(angle);
        float z = r * sin(angle);
        v.push_back({ {x, -h / 2, z}, {cos(angle), 0, sin(angle)}, {(float)i / segments, 0} });
        v.push_back({ {x,  h / 2, z}, {cos(angle), 0, sin(angle)}, {(float)i / segments, 1} });
    }
    // Top and bottom faces (simplified)
    v.push_back({ {0, -h / 2, 0}, {0, -1, 0}, {0.5, 0.5} });
    v.push_back({ {0,  h / 2, 0}, {0,  1, 0}, {0.5, 0.5} });
    return v;
}

vector<unsigned int> genCylinderIndices(int segments) {
    vector<unsigned int> ind;
    for (int i = 0; i < segments; i++) {
        int a = i * 2;
        int b = a + 1;
        int c = (i + 1) * 2;
        int d = c + 1;
        ind.push_back(a); ind.push_back(b); ind.push_back(c);
        ind.push_back(b); ind.push_back(d); ind.push_back(c);
    }
    return ind;
}

// ==================== توليد القرص (للبوابة السحرية) ====================
vector<Vertex> genDiscVertices(float r, int segments = 32) {
    vector<Vertex> v;
    v.push_back({ {0, 0, 0}, {0, 1, 0}, {0.5, 0.5} });
    for (int i = 0; i <= segments; i++) {
        float angle = 2 * 3.14159265f * i / segments;
        float x = r * cos(angle);
        float z = r * sin(angle);
        v.push_back({ {x, 0, z}, {0, 1, 0}, {(cos(angle) + 1) / 2, (sin(angle) + 1) / 2} });
    }
    return v;
}

vector<unsigned int> genDiscIndices(int segments) {
    vector<unsigned int> ind;
    for (int i = 1; i <= segments; i++) {
        ind.push_back(0);
        ind.push_back(i);
        ind.push_back(i + 1);
    }
    return ind;
}

// ==================== توليد الأرضية ====================
vector<Vertex> genPlaneVertices() {
    return {
        {{-15, -0.5f, -15}, {0,1,0}, {0,0}},
        {{ 15, -0.5f, -15}, {0,1,0}, {15,0}},
        {{ 15, -0.5f,  15}, {0,1,0}, {15,15}},
        {{-15, -0.5f,  15}, {0,1,0}, {0,15}}
    };
}
vector<unsigned int> genPlaneIndices() { return { 0,1,2, 0,2,3 }; }

// ==================== إنشاء Mesh ====================
unsigned int createMesh(const vector<Vertex>& verts, const vector<unsigned int>& inds, unsigned int& indexCount) {
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, norm));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex));

    glBindVertexArray(0);
    indexCount = inds.size();
    return VAO;
}

// ==================== تحميل النسيج ====================
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    return textureID;
}

// ==================== نسيج سحري متدرج ====================
unsigned int createMagicTexture() {
    unsigned int tex;
    glGenTextures(1, &tex);
    int w = 256, h = 256;
    unsigned char* data = new unsigned char[w * h * 3];
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int idx = (i * w + j) * 3;
            float r = sin(i * 0.05f) * 0.5f + 0.5f;
            float g = cos(j * 0.05f) * 0.5f + 0.5f;
            float b = sin((i + j) * 0.03f) * 0.5f + 0.5f;
            data[idx] = (unsigned char)(r * 255);
            data[idx + 1] = (unsigned char)(g * 255);
            data[idx + 2] = (unsigned char)(b * 255);
        }
    }
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    delete[] data;
    return tex;
}

// ==================== شادر OpenGL مع تأثيرات خاصة ====================
const char* vertexShaderSrc = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform float time;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out float VertexTime;
void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    VertexTime = time;
    gl_Position = proj * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in float VertexTime;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 objColor;
uniform sampler2D texture1;
uniform bool useTexture;
uniform float fogDensity;
uniform float time;
uniform bool isDream;
uniform bool isPortal;
uniform float pulseIntensity;
void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 ambient = 0.25 * lightColor;
    vec3 diffuse = diff * lightColor;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDirNorm, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.6 * spec * lightColor;
    vec3 result = (ambient + diffuse + specular);
    
    vec4 texColor = vec4(objColor, 1.0);
    if (useTexture) texColor = texture(texture1, TexCoord);
    
    // تأثير خاص للأحلام (توهج ونبض)
    if (isDream) {
        float glow = 0.8 + 0.4 * sin(time * 5.0);
        texColor.rgb = vec3(0.3, 0.6, 1.0) * (1.0 + glow * 0.5);
    }
    
    // تأثير خاص للبوابات (ألوان متغيرة)
    if (isPortal) {
        float r = 0.5 + 0.5 * sin(time * 2.0);
        float g = 0.3 + 0.5 * sin(time * 2.5);
        float b = 0.8 + 0.2 * sin(time * 3.0);
        texColor.rgb = vec3(r, g, b) * (0.8 + pulseIntensity * 0.5);
    }
    
    vec3 finalColor = result * texColor.rgb;
// ضباب سحري متدرج الألوان
    float dist = length(viewPos - FragPos);
    float fogFactor = exp(-pow(fogDensity * dist, 1.8));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    vec3 fogColor = vec3(0.4 + 0.1 * sin(time), 0.3 + 0.1 * cos(time * 0.7), 0.5 + 0.1 * sin(time * 0.5));
    finalColor = mix(fogColor, finalColor, fogFactor);
    
    FragColor = vec4(finalColor, 1.0);
}
)";

unsigned int compileShader(unsigned int type, const char* src) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        cout << "Shader error: " << log << endl;
    }
    return shader;
}

unsigned int createShaderProgram() {
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    int success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        cout << "Program error: " << log << endl;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// ==================== الكاميرا ====================
struct Camera {
    glm::vec3 target;
    float dist, yaw, pitch;
    float shakeAmount;
    float shakeTime;

    Camera(glm::vec3 t) : target(t), dist(CAM_DIST), yaw(-45.0f), pitch(25.0f), shakeAmount(0), shakeTime(0) {}

    void startShake(float amount) {
        shakeAmount = amount;
        shakeTime = 0.3f;
    }

    glm::mat4 getViewMatrix(float deltaTime) {
        if (shakeTime > 0) {
            shakeTime -= deltaTime;
            float currentShake = shakeAmount * (shakeTime / 0.3f);
            target.x += (rand() % 100 - 50) / 500.0f * currentShake;
            target.z += (rand() % 100 - 50) / 500.0f * currentShake;
        }

        glm::vec3 pos = target + glm::vec3(
            dist * cos(glm::radians(pitch)) * cos(glm::radians(yaw)),
            dist * sin(glm::radians(pitch)) + 1.5f,
            dist * cos(glm::radians(pitch)) * sin(glm::radians(yaw)));
        return glm::lookAt(pos, target, glm::vec3(0, 1, 0));
    }

    void rotate(float dy, float dp) {
        yaw += dy * SENSITIVITY;
        pitch += dp * SENSITIVITY;
        pitch = glm::clamp(pitch, -80.0f, 80.0f);
    }

    glm::vec3 getForward() {
        glm::vec3 f;
        f.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        f.y = sin(glm::radians(pitch));
        f.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        return glm::normalize(f);
    }
};

// ==================== اللاعب ====================
struct Player {
    glm::vec3 pos;
    float radius = PLAYER_RADIUS;
    bool canMove = true;
    glm::vec3 vel;

    void update(float dt, const vector<glm::vec3>& obstacles) {
        if (!canMove) return;
        glm::vec3 newPos = pos + vel * dt;
        for (auto& obs : obstacles) {
            if (glm::distance(newPos, obs) < radius + 0.5f) return;
        }
        newPos.x = glm::clamp(newPos.x, -12.0f + radius, 12.0f - radius);
        newPos.z = glm::clamp(newPos.z, -12.0f + radius, 12.0f - radius);
        pos = newPos;
    }
};

// ==================== الحلم ====================
struct Dream {
    glm::vec3 pos;
    bool collected = false;
    float angle = 0;
    float floatY = 0;
    float floatDir = 1;

    void update(float dt) {
        angle += dt * 3.0f;
        floatY += floatDir * dt * 1.2f;
        if (floatY > 0.4f || floatY < -0.1f) floatDir *= -1;
    }
};
// ==================== العين الساحرة ====================
struct EvilEye {
    glm::vec3 pos;
    float speed;
    glm::vec3 direction;
    float pupilAngle;

    EvilEye(glm::vec3 p, float s) : pos(p), speed(s), pupilAngle(0) {
        direction = glm::normalize(glm::vec3((rand() % 200 - 100) / 100.0f, 0, (rand() % 200 - 100) / 100.0f));
    }

    void update(float dt, const glm::vec3& playerPos) {
        // تتحرك العين باتجاه اللاعب بذكاء
        glm::vec3 toPlayer = playerPos - pos;
        toPlayer.y = 0;
        if (glm::length(toPlayer) > 0.1f) {
            direction = glm::normalize(direction * 0.95f + toPlayer * 0.05f);
        }
        pos += direction * speed * dt;

        // ارتداد عند الحدود
        if (pos.x > 11 || pos.x < -11) direction.x *= -1;
        if (pos.z > 11 || pos.z < -11) direction.z *= -1;

        pupilAngle += dt * 4.0f;
    }
};

// ==================== البرج السحري ====================
struct MagicTower {
    glm::vec3 pos;
    float orbAngle;

    MagicTower(glm::vec3 p) : pos(p), orbAngle(0) {}

    void update(float dt) {
        orbAngle += dt * 3.0f;
    }
};

// ==================== بوابة سحرية ====================
struct Portal {
    glm::vec3 pos;
    PortalState state;
    float activationTimer;
    float pulseScale;

    Portal(glm::vec3 p) : pos(p), state(INACTIVE), activationTimer(0), pulseScale(0) {}

    void update(float dt, const glm::vec3& playerPos) {
        if (state == ACTIVATING) {
            activationTimer += dt;
            pulseScale = activationTimer / 2.0f;
            if (activationTimer >= 2.0f) {
                state = ACTIVE;
                activationTimer = 0;
            }
        }
        else if (state == ACTIVE) {
            pulseScale = 0.5f + sin(activationTimer * 8) * 0.3f;
            activationTimer += dt;
        }
    }

    bool tryActivate(const glm::vec3& playerPos) {
        if (state == INACTIVE && glm::distance(playerPos, pos) < PORTAL_RADIUS) {
            state = ACTIVATING;
            return true;
        }
        return false;
    }
};

// ==================== MAIN ====================
int main() {
    srand(time(0));

    // تهيئة GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "The Stolen Dreams Castle - Epic 3D Adventure", nullptr, nullptr);
    if (!window) {
        cout << "Failed to create window\n";
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // إنشاء الشادر
    unsigned int shaderProg = createShaderProgram();
    glUseProgram(shaderProg);

    // إعدادات الإضاءة والضباب
    glUniform3f(glGetUniformLocation(shaderProg, "lightDir"), 0.5f, -1.5f, 0.8f);
    glUniform3f(glGetUniformLocation(shaderProg, "lightColor"), 1.0f, 0.85f, 0.7f);
    glUniform1f(glGetUniformLocation(shaderProg, "fogDensity"), 0.04f);

    glm::mat4 proj = glm::perspective(glm::radians(55.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProg, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

    // إنشاء الموشات
    auto cubeVerts = genCubeVertices();
    auto cubeInds = genCubeIndices();
    unsigned int cubeIndexCount;
    unsigned int cubeVAO = createMesh(cubeVerts, cubeInds, cubeIndexCount);
    auto sphereVerts = genSphereVertices(0.5f);
    auto sphereInds = genSphereIndices(32, 32);
    unsigned int sphereIndexCount;
    unsigned int sphereVAO = createMesh(sphereVerts, sphereInds, sphereIndexCount);

    auto cylinderVerts = genCylinderVertices(0.6f, 1.8f);
    auto cylinderInds = genCylinderIndices(24);
    unsigned int cylinderIndexCount;
    unsigned int cylinderVAO = createMesh(cylinderVerts, cylinderInds, cylinderIndexCount);

    auto discVerts = genDiscVertices(0.8f);
    auto discInds = genDiscIndices(32);
    unsigned int discIndexCount;
    unsigned int discVAO = createMesh(discVerts, discInds, discIndexCount);

    auto planeVerts = genPlaneVertices();
    auto planeInds = genPlaneIndices();
    unsigned int planeIndexCount;
    unsigned int planeVAO = createMesh(planeVerts, planeInds, planeIndexCount);

    // القواميس
    unsigned int floorTex = createMagicTexture();
    unsigned int goldTex = createMagicTexture();

    // ==================== إنشاء عالم اللعبة السحري ====================
    Player player;
    player.pos = glm::vec3(0.0f, 0.0f, 0.0f);

    Camera camera(player.pos);

    // الأحلام المسروقة (3 أحلام في مواقع إستراتيجية)
    vector<Dream> dreams;
    dreams.push_back(Dream{ {-7.0f, 0.3f, -6.0f} });
    dreams.push_back(Dream{ { 6.0f, 0.3f, -5.0f} });
    dreams.push_back(Dream{ { 0.0f, 0.3f,  8.0f} });

    // العيون الساحرة (5 عيون تطارد اللاعب)
    vector<EvilEye> evilEyes;
    evilEyes.push_back(EvilEye({ 4.0f, 0.3f, 3.0f }, 2.5f));
    evilEyes.push_back(EvilEye({ -4.0f, 0.3f, 4.0f }, 2.0f));
    evilEyes.push_back(EvilEye({ 5.0f, 0.3f, -3.0f }, 3.0f));
    evilEyes.push_back(EvilEye({ -5.0f, 0.3f, -4.0f }, 2.2f));
    evilEyes.push_back(EvilEye({ 0.0f, 0.3f, 5.0f }, 1.8f));

    // الأبراج السحرية (عوائق ثابتة مع كرات دوارة)
    vector<MagicTower> towers;
    towers.push_back(MagicTower({ -3, -0.2f, -4 }));
    towers.push_back(MagicTower({ 3, -0.2f, -4 }));
    towers.push_back(MagicTower({ -4, -0.2f,  3 }));
    towers.push_back(MagicTower({ 4, -0.2f,  3 }));
    towers.push_back(MagicTower({ -2, -0.2f,  6 }));
    towers.push_back(MagicTower({ 2, -0.2f,  6 }));

    // البوابات السحرية (بوابة لكل حلم)
    vector<Portal> portals;
    portals.push_back(Portal({ -7.0f, 0.0f, -6.0f }));
    portals.push_back(Portal({ 6.0f, 0.0f, -5.0f }));
    portals.push_back(Portal({ 0.0f, 0.0f,  8.0f }));

    // مواقع الأبراج للتصادم
    vector<glm::vec3> towerPositions;
    for (auto& t : towers) towerPositions.push_back(t.pos);

    int dreamsCollected = 0;
    int totalDreams = dreams.size();
    int gameState = PLAYING;

    // متغيرات المؤثرات
    float gameTime = 0;
    float pulseIntensity = 0;

    // متغيرات التحكم
    float lastFrame = 0.0f;
    float lastX = SCR_WIDTH / 2, lastY = SCR_HEIGHT / 2;
    bool firstMouse = true;
    bool rPressed = false;
    bool spacePressed = false;

    while (!glfwWindowShouldClose(window)) {
        float now = glfwGetTime();
        float dt = now - lastFrame;
        lastFrame = now;
        gameTime += dt;
        if (dt > 0.033f) dt = 0.033f;

        // تحديث نبض البوابة
        pulseIntensity = sin(gameTime * 3) * 0.3f + 0.7f;

        // ==================== معالجة المدخلات ====================
        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) break;

        // إعادة التشغيل
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rPressed && gameState != PLAYING) {
            player.pos = glm::vec3(0.0f);
            dreamsCollected = 0;
            gameState = PLAYING;
            player.canMove = true;
            for (auto& d : dreams) d.collected = false;
            for (auto& p : portals) p.state = INACTIVE;
            rPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) rPressed = false;
        // حركة اللاعب
        if (gameState == PLAYING) {
            glm::vec3 forward = camera.getForward();
            forward.y = 0;
            forward = glm::normalize(forward);
            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
            glm::vec3 move(0);
            if (glfwGetKey(window, GLFW_KEY_W)) move += forward;
            if (glfwGetKey(window, GLFW_KEY_S)) move -= forward;
            if (glfwGetKey(window, GLFW_KEY_D)) move += right;
            if (glfwGetKey(window, GLFW_KEY_A)) move -= right;
            if (glm::length(move) > 0.1f) move = glm::normalize(move);
            player.vel = move * PLAYER_SPEED;

            // تحديث العيون الساحرة
            for (auto& e : evilEyes) e.update(dt, player.pos);

            // تحديث الأبراج
            for (auto& t : towers) t.update(dt);

            // تحديث الأحلام والبوابات
            for (int i = 0; i < dreams.size(); i++) {
                dreams[i].update(dt);
                portals[i].update(dt, player.pos);

                // تفعيل البوابة بالوقوف عليها
                if (!dreams[i].collected && portals[i].state == INACTIVE) {
                    if (portals[i].tryActivate(player.pos)) {
                        camera.startShake(0.5f);
                    }
                }

                // إذا البوابة نشطة، اجمع الحلم
                if (!dreams[i].collected && portals[i].state == ACTIVE && glm::distance(player.pos, dreams[i].pos) < DREAM_RADIUS + PLAYER_RADIUS) {
                    dreams[i].collected = true;
                    dreamsCollected++;
                    camera.startShake(0.8f);
                }
            }

            // فحص التصادم مع العيون (خسارة)
            for (auto& e : evilEyes) {
                if (glm::distance(player.pos, e.pos) < PLAYER_RADIUS + EYE_RADIUS) {
                    gameState = LOSE;
                    player.canMove = false;
                }
            }

            // فحص الفوز
            if (dreamsCollected >= totalDreams) {
                gameState = WIN;
                player.canMove = false;
            }

            // تحديث اللاعب مع الأبراج
            player.update(dt, towerPositions);
            camera.target = player.pos;
        }

        // ==================== دوران الكاميرا ====================
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        if (firstMouse) {
            lastX = mx; lastY = my;
            firstMouse = false;
        }
        camera.rotate(mx - lastX, lastY - my);
        lastX = mx; lastY = my;

        // ==================== عرض المعلومات ====================
        string title = "✨ THE STOLEN DREAMS CASTLE ✨ - Dreams: " + to_string(dreamsCollected) + "/" + to_string(totalDreams);
        if (gameState == WIN) title += " - VICTORY! The dreams are free! Press R";
        else if (gameState == LOSE) title += " - DEFEAT! The eyes caught you! Press R";
        else title += " - Stand on MAGIC PORTALS to collect dreams! Avoid EVIL EYES!";
        glfwSetWindowTitle(window, title.c_str());

        // ==================== الرسم ====================
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProg);

        // إعدادات الـ shader
        glUniform1f(glGetUniformLocation(shaderProg, "time"), gameTime);
        glUniform1f(glGetUniformLocation(shaderProg, "pulseIntensity"), pulseIntensity);

        glm::mat4 view = camera.getViewMatrix(dt);
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shaderProg, "viewPos"), camera.target.x, camera.target.y, camera.target.z);
        // الأرضية السحرية
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.5f, 0));
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(shaderProg, "objColor"), 0.3f, 0.25f, 0.35f);
        glUniform1i(glGetUniformLocation(shaderProg, "useTexture"), 1);
        glUniform1i(glGetUniformLocation(shaderProg, "isDream"), 0);
        glUniform1i(glGetUniformLocation(shaderProg, "isPortal"), 0);
        glBindTexture(GL_TEXTURE_2D, floorTex);
        glBindVertexArray(planeVAO);
        glDrawElements(GL_TRIANGLES, planeIndexCount, GL_UNSIGNED_INT, 0);

        // الأبراج السحرية
        glUniform1i(glGetUniformLocation(shaderProg, "useTexture"), 0);
        glUniform1i(glGetUniformLocation(shaderProg, "isDream"), 0);
        glUniform1i(glGetUniformLocation(shaderProg, "isPortal"), 0);
        for (auto& t : towers) {
            model = glm::translate(glm::mat4(1.0f), t.pos);
            glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(glGetUniformLocation(shaderProg, "objColor"), 0.4f, 0.35f, 0.5f);
            glBindVertexArray(cylinderVAO);
            glDrawElements(GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_INT, 0);

            // الكرات الدوارة حول الأبراج
            float orbX = t.pos.x + 0.8f * cos(t.orbAngle);
            float orbZ = t.pos.z + 0.8f * sin(t.orbAngle);
            glm::mat4 orbModel = glm::translate(glm::mat4(1.0f), glm::vec3(orbX, t.pos.y + 1.0f, orbZ));
            orbModel = glm::scale(orbModel, glm::vec3(0.2f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(orbModel));
            glUniform3f(glGetUniformLocation(shaderProg, "objColor"), 0.8f, 0.5f, 0.2f);
            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        }

        // العيون الساحرة (تطارد اللاعب)
        for (auto& e : evilEyes) {
            model = glm::translate(glm::mat4(1.0f), e.pos);
            model = glm::scale(model, glm::vec3(0.4f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(glGetUniformLocation(shaderProg, "objColor"), 0.9f, 0.15f, 0.2f);
            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);

            // بؤبؤ العين
            float pupilX = 0.15f * cos(e.pupilAngle);
            float pupilZ = 0.15f * sin(e.pupilAngle);
            glm::mat4 pupilModel = glm::translate(glm::mat4(1.0f), e.pos + glm::vec3(pupilX, 0.1f, pupilZ));
            pupilModel = glm::scale(pupilModel, glm::vec3(0.15f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(pupilModel));
            glUniform3f(glGetUniformLocation(shaderProg, "objColor"), 0.0f, 0.0f, 0.0f);
            glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        }

        // البوابات السحرية
        for (int i = 0; i < portals.size(); i++) {
            if (dreams[i].collected) continue;
            model = glm::translate(glm::mat4(1.0f), portals[i].pos);
            model = glm::rotate(model, gameTime, glm::vec3(0, 1, 0));
            float scale = 0.8f;
            if (portals[i].state == ACTIVATING) scale = 0.8f + portals[i].pulseScale * 0.5f;
            else if (portals[i].state == ACTIVE) scale = 0.8f + portals[i].pulseScale * 0.3f;
            model = glm::scale(model, glm::vec3(scale, 0.05f, scale));
            glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(glGetUniformLocation(shaderProg, "isPortal"), 1);
            glUniform1i(glGetUniformLocation(shaderProg, "useTexture"), 0);
            glBindVertexArray(discVAO);
            glDrawElements(GL_TRIANGLES, discIndexCount, GL_UNSIGNED_INT, 0);
        }

        // الأحلام المسروقة (تتوهج وتطفو)
        glUniform1i(glGetUniformLocation(shaderProg, "isDream"), 1);
        glUniform1i(glGetUniformLocation(shaderProg, "isPortal"), 0);
        for (auto& d : dreams) {
            if (d.collected) continue;
            model = glm::translate(glm::mat4(1.0f), d.pos + glm::vec3(0, d.floatY, 0));
            model = glm::rotate(model, d.angle, glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(0.35f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(glGetUniformLocation(shaderProg, "objColor"), 0.4f, 0.6f, 1.0f);
            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);

            // تأثير الهالة حول الحلم
            model = glm::translate(glm::mat4(1.0f), d.pos + glm::vec3(0, d.floatY, 0));
            model = glm::scale(model, glm::vec3(0.55f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(glGetUniformLocation(shaderProg, "objColor"), 0.5f, 0.7f, 1.0f);
            glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        }

        // اللاعب (شخصية ساحرة)
        glUniform1i(glGetUniformLocation(shaderProg, "isDream"), 0);
        model = glm::translate(glm::mat4(1.0f), player.pos);
        model = glm::scale(model, glm::vec3(0.65f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(shaderProg, "objColor"), 0.2f, 0.6f, 0.9f);
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, cubeIndexCount, GL_UNSIGNED_INT, 0);

        // رأس اللاعب
        model = glm::translate(glm::mat4(1.0f), player.pos + glm::vec3(0, 0.7f, 0));
        model = glm::scale(model, glm::vec3(0.45f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(shaderProg, "objColor"), 1.0f, 0.85f, 0.7f);
        glDrawElements(GL_TRIANGLES, cubeIndexCount, GL_UNSIGNED_INT, 0);

        // تاج اللاعب (دائرة ذهبية)
        model = glm::translate(glm::mat4(1.0f), player.pos + glm::vec3(0, 0.95f, 0));
        model = glm::scale(model, glm::vec3(0.55f, 0.1f, 0.55f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(shaderProg, "objColor"), 1.0f, 0.7f, 0.2f);
        glDrawElements(GL_TRIANGLES, cubeIndexCount, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}