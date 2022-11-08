#include "lab_m2/lab5/lab5.h"

#include <vector>
#include <iostream>

using namespace std;
using namespace m2;


struct Particle
{
    glm::vec4 position;
    glm::vec4 speed;
    glm::vec4 initialPos;
    glm::vec4 initialSpeed;

    Particle() {}

    Particle(const glm::vec4 &pos, const glm::vec4 &speed)
    {
        SetInitial(pos, speed);
    }

    void SetInitial(const glm::vec4 &pos, const glm::vec4 &speed)
    {
        position = pos;
        initialPos = pos;

        this->speed = speed;
        initialSpeed = speed;
    }
};


ParticleEffect<Particle> *particleEffect;
ParticleEffect<Particle> *particleEffect_smoke;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Lab5::Lab5()
{
}


Lab5::~Lab5()
{
}


void Lab5::Init()
{
    dt_fire = 0.0;
    dt_smoke = 0.0;

    ds_fire = 0.02;
    ds_smoke = 0.02;

    dh_fire = 100;
    dh_smoke = 100;

    direction = 1;

    auto camera = GetSceneCamera();
    camera->SetPositionAndRotation(glm::vec3(0, 8, 8), glm::quat(glm::vec3(-40 * TO_RADIANS, 0, 0)));
    camera->Update();

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    // Load textures
    {
        TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "fire_particle.png");
    }
    {
        TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "smoke_particle.png");
    }

    LoadShader("Simple", false);
    LoadShader("Particle");
    LoadShader("Smoke_particle");

    // number of particles
    unsigned int nrParticles = 50000;

    particleEffect = new ParticleEffect<Particle>();
    particleEffect->Generate(nrParticles, true);

    auto particleSSBO = particleEffect->GetParticleBuffer();
    Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());

    sphereDiametre = 4;
    rSize = sphereDiametre / 2;

    for (unsigned int i = 0; i < nrParticles; i++)
    {
        // aici creez sfera de unde pleaca particulele
        glm::vec4 pos(1);
        float z = (rand() / static_cast<float>(RAND_MAX)) * sphereDiametre - rSize;
        float theta = glm::asin(z/rSize);
        float phi = (rand() / static_cast<float>(RAND_MAX)) * (2 * 3.14);
        pos.x = rSize * cos(phi) * cos(theta);
        pos.y = rSize * sin(phi) * cos(theta);
        pos.z = z;

        glm::vec4 speed(0);
        speed.x = (rand() % 20 - 10) / 10.0f;
        speed.z = (rand() % 20 - 10) / 10.0f;
        speed.y = rand() % 2 + 2.0f;

        //cout << pos.x << " " << pos.y << " " << pos.z << endl;
 
        data[i].SetInitial(pos, speed);
    }

    particleSSBO->SetBufferData(data);

    unsigned int nrSmokeParticles = 1000;

    particleEffect_smoke = new ParticleEffect<Particle>();
    particleEffect_smoke->Generate(nrSmokeParticles, true);

    auto particleSSBO_smoke = particleEffect_smoke->GetParticleBuffer();
    Particle* data_smoke = const_cast<Particle*>(particleSSBO_smoke->GetBuffer());

    smokeSphereDiametre = 5;
    rSmokeSize = smokeSphereDiametre / 2;

    for (unsigned int i = 0; i < nrSmokeParticles; i++)
    {
        // aici creez sfera de unde pleaca particulele
        glm::vec4 pos(1);
        float z = (rand() / static_cast<float>(RAND_MAX)) * smokeSphereDiametre - rSmokeSize;
        float theta = glm::asin(z / rSmokeSize);
        float phi = (rand() / static_cast<float>(RAND_MAX)) * (2 * 3.14);
        pos.x = rSmokeSize * cos(phi) * cos(theta);
        pos.y = rSmokeSize * sin(phi) * cos(theta);
        if (pos.y < 2)
            pos.y += 3;
        pos.z = z;

        glm::vec4 speed(0);
        speed.x = (rand() % 20 - 10) / 10.0f;
        speed.z = (rand() % 20 - 10) / 10.0f;
        speed.y = rand() % 2 + 2.0f;

        //cout << pos.x << " " << pos.y << " " << pos.z << endl;

        data_smoke[i].SetInitial(pos, speed);
    }

    particleSSBO_smoke->SetBufferData(data_smoke);
}


void Lab5::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Lab5::Update(float deltaTimeSeconds)
{
    glLineWidth(3);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    {
        auto shader = shaders["Particle"];
        if (shader->GetProgramID())
        {
            shader->Use();

            int loc_dt = shader->GetUniformLocation("dt");
            glUniform1f(loc_dt, dt_fire);

            int loc_ds = shader->GetUniformLocation("ds");
            glUniform1f(loc_ds, ds_fire);

            int loc_dh = shader->GetUniformLocation("dh");
            glUniform1i(loc_dh, dh_fire);

            int loc_dir = shader->GetUniformLocation("direction");
            glUniform1i(loc_dir, direction);

            TextureManager::GetTexture("fire_particle.png")->BindToTextureUnit(GL_TEXTURE0);
            particleEffect->Render(GetSceneCamera(), shader);
        }
    }
    {
        auto shader_smoke = shaders["Smoke_particle"];
        if (shader_smoke->GetProgramID())
        {
            shader_smoke->Use();

            int loc_dt = shader_smoke->GetUniformLocation("dt");
            glUniform1f(loc_dt, dt_smoke);

            int loc_ds = shader_smoke->GetUniformLocation("ds");
            glUniform1f(loc_ds, ds_smoke);

            int loc_dh = shader_smoke->GetUniformLocation("dh");
            glUniform1i(loc_dh, dh_smoke);

            int loc_dir = shader_smoke->GetUniformLocation("direction");
            glUniform1i(loc_dir, direction);

            TextureManager::GetTexture("smoke_particle.png")->BindToTextureUnit(GL_TEXTURE0);
            particleEffect_smoke->Render(GetSceneCamera(), shader_smoke);
        }
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    {
       /* glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(3, 0, 0));*/
        //RenderMesh(meshes["sphere"], shaders["Simple"], glm::mat4(1));
    }
}


void Lab5::FrameEnd()
{
#if 0
    DrawCoordinateSystem();
#endif
}


void Lab5::LoadShader(const std::string &name, bool hasGeomtery)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "lab5", "shaders");

    // Create a shader program for particle system
    {
        Shader *shader = new Shader(name);
        shader->AddShader(PATH_JOIN(shaderPath, name + ".VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, name + ".FS.glsl"), GL_FRAGMENT_SHADER);
        if (hasGeomtery)
        {
            shader->AddShader(PATH_JOIN(shaderPath, name + ".GS.glsl"), GL_GEOMETRY_SHADER);
        }

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab5::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
}


void Lab5::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_EQUAL) {
        dt_fire+=0.01;
        cout << "dt_fire: " << dt_fire << endl;
    }
    if (key == GLFW_KEY_MINUS) {
        dt_fire-= 0.01;
        cout << "dt_fire: " << dt_fire << endl;
    }

    if (key == GLFW_KEY_RIGHT_BRACKET) {
        dt_smoke += 0.01;
        cout << "dt_smoke: " << dt_smoke << endl;
    }
    if (key == GLFW_KEY_LEFT_BRACKET) {
        dt_smoke -= 0.01;
        cout << "dt_smoke: " << dt_smoke << endl;
    }

    if (key == GLFW_KEY_0) {
        ds_fire += 0.01;
        cout << "ds_fire: " << ds_fire << endl;
    }
    if (key == GLFW_KEY_9 && ds_fire >= 0.03) {
        ds_fire -= 0.01;
        cout << "ds_fire: " << ds_fire << endl;
    }

    if (key == GLFW_KEY_P) {
        ds_smoke += 0.01;
        cout << "ds_smoke: " << ds_smoke << endl;
    }
    if (key == GLFW_KEY_O && ds_smoke >= 0.03) {
        ds_smoke -= 0.01;
        cout << "ds_smoke: " << ds_smoke << endl;
    }

    if (key == GLFW_KEY_8) {
        dh_fire += 100;
        cout << "dh_fire: " << dh_fire << endl;
    }
    if (key == GLFW_KEY_7 && dh_fire >= 100) {
        dh_fire -= 100;
        cout << "dh_fire: " << dh_fire << endl;
    }

    if (key == GLFW_KEY_I) {
        dh_smoke += 100;
        cout << "dh_smoke: " << dh_smoke << endl;
    }
    if (key == GLFW_KEY_U && dh_smoke >= 100) {
        dh_smoke -= 100;
        cout << "dh_smoke: " << dh_smoke << endl;
    }

    if (key == GLFW_KEY_1 && direction != 1) {
        direction = 1;
        cout << "direction: " << direction << endl;
    }
    if (key == GLFW_KEY_2 && direction != 2) {
        direction = 2;
        cout << "direction: " << direction << endl;
    }
    if (key == GLFW_KEY_3 && direction != 3) {
        direction = 3;
        cout << "direction: " << direction << endl;
    }
    if (key == GLFW_KEY_4 && direction != 4) {
        direction = 4;
        cout << "direction: " << direction << endl;
    }
    if (key == GLFW_KEY_5 && direction != 5) {
        direction = 5;
        cout << "direction: " << direction << endl;
    }
}


void Lab5::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab5::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Lab5::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Lab5::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab5::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Lab5::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
