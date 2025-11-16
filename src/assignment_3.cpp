#include <cstdlib>
#include <iostream>

#include "mygl/camera.h"
#include "mygl/geometry.h"
#include "mygl/mesh.h"
#include "mygl/shader.h"

#include "ground.h"

/* struct holding all necessary state variables for scene */
struct {
    /* camera */
    Camera camera;
    bool cameraFollowPickup;
    float zoomSpeedMultiplier;

    /* game objects */
    Ground ground;

    /* cubes */
    Mesh baseCarMesh;
    Mesh windowCarMesh;

    /* cylinders */
    Mesh bottomLeftWheelMesh;
    Mesh bottomRightWheelMesh;
    Mesh topLeftWheelMesh;
    Mesh topRightWheelMesh;
    Mesh spareWheelMesh;

    /* transformation matrices */

    /* car */

    Matrix4D carTransformationMatrix;

    /* car components */

    /* cubes */
    Matrix4D baseCarScalingMatrix;
    Matrix4D baseCarTranslationMatrix;
    Matrix4D baseCarTransformationMatrix;

    Matrix4D windowCarScalingMatrix;
    Matrix4D windowCarTranslationMatrix;
    Matrix4D windowCarTransformationMatrix;

    /* cylinders */
    Matrix4D bottomLeftWheelScalingMatrix;
    Matrix4D bottomLeftWheelTranslationMatrix;
    Matrix4D bottomLeftWheelTransformationMatrix;

    Matrix4D bottomRightWheelScalingMatrix;
    Matrix4D bottomRightWheelTranslationMatrix;
    Matrix4D bottomRightWheelTransformationMatrix;

    Matrix4D topLeftWheelScalingMatrix;
    Matrix4D topLeftWheelTranslationMatrix;
    Matrix4D topLeftWheelTransformationMatrix;

    Matrix4D topRightWheelScalingMatrix;
    Matrix4D topRightWheelTranslationMatrix;
    Matrix4D topRightWheelTransformationMatrix;

    Matrix4D spareWheelScalingMatrix;
    Matrix4D spareWheelTranslationMatrix;
    Matrix4D spareWheelTransformationMatrix;

    float cubeSpinRadPerSecond;
    float speed;

    float frontWheelSpinAccumulator;

    /* shader */
    ShaderProgram shaderColor;
} sScene;

/* calculate how much the car approximately turns per meter travelled for a given turning angle */
float calculateTurningAnglePerMeter(float wheelBase, float turningAngle, float width) {
    /* according to https://calculator.academy/turning-radius-calculator/ and
     * https://gamedev.stackexchange.com/questions/50022/typical-maximum-steering-angle-of-a-real-car/50029 */
    float turningRadius = wheelBase / tan(turningAngle);
    float innerTurningRadius = turningRadius - width;
    return 360.0f / (2.0f * innerTurningRadius * static_cast<float>(M_PI));
}

/* struct holding all state variables for input */
struct {
    bool mouseLeftButtonPressed = false;
    Vector2D mousePressStart;
    bool buttonPressed[4] = {false, false, false, false};
} sInput;

/* GLFW callback function for keyboard events */
void callbackKey(GLFWwindow *window, int key, int scancode, int action, int mods) {
    /* called on keyboard event */

    /* close window on escape */
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    /* make screenshot and save in work directory */
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        screenshotToPNG("screenshot.png");
    }

    /* input for car control */
    if (key == GLFW_KEY_W) {
        sInput.buttonPressed[0] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_S) {
        sInput.buttonPressed[1] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if (key == GLFW_KEY_A) {
        sInput.buttonPressed[2] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_D) {
        sInput.buttonPressed[3] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
}

/* GLFW callback function for mouse position events */
void callbackMousePos(GLFWwindow *window, double x, double y) {
    /* called on cursor position change */
    if (sInput.mouseLeftButtonPressed) {
        Vector2D diff = sInput.mousePressStart - Vector2D(x, y);
        cameraUpdateOrbit(sScene.camera, diff, 0.0f);
        sInput.mousePressStart = Vector2D(x, y);
    }
}

/* GLFW callback function for mouse button events */
void callbackMouseButton(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        sInput.mouseLeftButtonPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        sInput.mousePressStart = Vector2D(x, y);
    }
}

/* GLFW callback function for mouse scroll events */
void callbackMouseScroll(GLFWwindow *window, double xoffset, double yoffset) {
    cameraUpdateOrbit(sScene.camera, {0, 0}, -sScene.zoomSpeedMultiplier * yoffset);
}

/* GLFW callback function for window resize event */
void callbackWindowResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    sScene.camera.width = width;
    sScene.camera.height = height;
}

/* function to setup and initialize the whole scene */
void sceneInit(float width, float height) {

    /* initialize camera */
    sScene.camera = cameraCreate(width, height, to_radians(45.0f), 0.01f, 500.0f, {12.0f, 4.0f, -12.0f});
    sScene.cameraFollowPickup = false;
    sScene.zoomSpeedMultiplier = 0.05f;

    /* setup objects in scene and create opengl buffers for meshes */
    sScene.ground = groundCreate({0.15f, 0.35f, 0.15f});

    /* car */
    sScene.carTransformationMatrix = Matrix4D::identity();

    /* cubes */
    sScene.baseCarMesh = meshCreate(cube::vertices, cube::indices,GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.windowCarMesh = meshCreate(cube::vertices, cube::indices,GL_STATIC_DRAW, GL_STATIC_DRAW);

    /* cylinders */
    sScene.bottomLeftWheelMesh = meshCreate(cylinder::vertexPos, cylinder::indices, {0.3f, 0.3f, 0.3f, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.bottomRightWheelMesh = meshCreate(cylinder::vertexPos, cylinder::indices, {0.3f, 0.3f, 0.3f, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.topLeftWheelMesh = meshCreate(cylinder::vertexPos, cylinder::indices, {0.3f, 0.3f, 0.3f, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.topRightWheelMesh = meshCreate(cylinder::vertexPos, cylinder::indices, {0.3f, 0.3f, 0.3f, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.spareWheelMesh = meshCreate(cylinder::vertexPos, cylinder::indices, {0.3f, 0.3f, 0.3f, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);

    /* setup transformation matrices for objects */

    /* origin of "3D-Model" */
    float baseX = 0.0f;
    float baseY = 1.0f;  // Half of 2.0f
    float baseZ = 0.0f;

    /* cubes */
    sScene.baseCarScalingMatrix = Matrix4D::scale(1.0f, 0.5f, 2.0f);  // Half of (2.0, 1.0, 4.0)
    sScene.baseCarTranslationMatrix = Matrix4D::translation({baseX, baseY, baseZ});
    sScene.baseCarTransformationMatrix = Matrix4D::identity();

    sScene.windowCarScalingMatrix = Matrix4D::scale(1.0f, 0.5f, 0.5f);  // Half of (2.0, 1.0, 1.0)
    sScene.windowCarTranslationMatrix = Matrix4D::translation({baseX, baseY + 1.0f, baseZ + 0.5f});  // Half offsets
    sScene.windowCarTransformationMatrix = Matrix4D::identity();

    /* cylinders */
    sScene.bottomLeftWheelScalingMatrix = Matrix4D::scale(0.1f, 0.5f, 0.5f);  // Half of (0.2, 1.0, 1.0)
    sScene.bottomLeftWheelTranslationMatrix = Matrix4D::translation({baseX + 1.1f, baseY - 0.5f, baseZ - 1.1f});  // Half offsets
    sScene.bottomLeftWheelTransformationMatrix = Matrix4D::identity();

    sScene.bottomRightWheelScalingMatrix = Matrix4D::scale(0.1f, 0.5f, 0.5f);  // Half of (0.2, 1.0, 1.0)
    sScene.bottomRightWheelTranslationMatrix = Matrix4D::translation({baseX - 1.1f, baseY - 0.5f, baseZ - 1.1f});  // Half offsets
    sScene.bottomRightWheelTransformationMatrix = Matrix4D::identity();

    sScene.topLeftWheelScalingMatrix = Matrix4D::scale(0.1f, 0.35f, 0.35f);  // Half of (0.2, 0.7, 0.7)
    sScene.topLeftWheelTranslationMatrix = Matrix4D::translation({baseX + 1.1f, baseY - 0.575f, baseZ + 1.3f});  // Half offsets
    sScene.topLeftWheelTransformationMatrix = Matrix4D::identity();

    sScene.topRightWheelScalingMatrix = Matrix4D::scale(0.1f, 0.35f, 0.35f);  // Half of (0.2, 0.7, 0.7)
    sScene.topRightWheelTranslationMatrix = Matrix4D::translation({baseX - 1.1f, baseY - 0.575f, baseZ + 1.3f});  // Half offsets
    sScene.topRightWheelTransformationMatrix = Matrix4D::identity();

    sScene.spareWheelScalingMatrix = Matrix4D::scale(0.1f, 0.35f, 0.35f);  // Half of (0.2, 0.7, 0.7)
    sScene.spareWheelTranslationMatrix = Matrix4D::translation({baseX, baseY + 0.3f, baseZ - 2.1f});  // Half offsets
    sScene.spareWheelTransformationMatrix = Matrix4D::Matrix4D::rotationY(M_PI / 2.0f);

    sScene.cubeSpinRadPerSecond = M_PI / 2.0f;

    sScene.speed = 4.0f;

    sScene.frontWheelSpinAccumulator = 0.0f;

    /* load shader from file */
    sScene.shaderColor = shaderLoad("../../src/shader/default.vert", "../../src/shader/default.frag");

}

// extracts the position vector from a transformation matrix
static Vector3D extractPosition(const Matrix4D &m) {
    return {
        m.n[3][0],
        m.n[3][1],
        m.n[3][2]
    };
}


void updateCarPosition(Matrix4D translation_matrix) {

    // update all components of the Car by the same translation-matrix
    sScene.baseCarTranslationMatrix = translation_matrix * sScene.baseCarTranslationMatrix;
    sScene.windowCarTranslationMatrix = translation_matrix * sScene.windowCarTranslationMatrix;
    sScene.bottomLeftWheelTranslationMatrix = translation_matrix * sScene.bottomLeftWheelTranslationMatrix;
    sScene.bottomRightWheelTranslationMatrix = translation_matrix * sScene.bottomRightWheelTranslationMatrix;
    sScene.topLeftWheelTranslationMatrix = translation_matrix * sScene.topLeftWheelTranslationMatrix;
    sScene.topRightWheelTranslationMatrix = translation_matrix * sScene.topRightWheelTranslationMatrix;
    sScene.spareWheelTranslationMatrix = translation_matrix * sScene.spareWheelTranslationMatrix;

    const float frontWheelRadius = 0.35f;  // Half of 0.7f
    const float rearWheelRadius  = 0.5f;   // Half of 1.0f

    // get the current positions of all four wheels
    Vector3D blCenter = extractPosition(sScene.bottomLeftWheelTranslationMatrix);
    Vector3D brCenter = extractPosition(sScene.bottomRightWheelTranslationMatrix);
    Vector3D flCenter = extractPosition(sScene.topLeftWheelTranslationMatrix);
    Vector3D frCenter = extractPosition(sScene.topRightWheelTranslationMatrix);

    // get the height of the ground at the position of each wheel
    float blHeight = groundGetHeightAt(sScene.ground, blCenter);
    float brHeight = groundGetHeightAt(sScene.ground, brCenter);
    float flHeight = groundGetHeightAt(sScene.ground, flCenter);
    float frHeight = groundGetHeightAt(sScene.ground, frCenter);

    // compute how much each wheel needs to be moved up/down to be on the ground
    float blDelta = (blHeight + rearWheelRadius)  - blCenter.y;
    float brDelta = (brHeight + rearWheelRadius)  - brCenter.y;
    float flDelta = (flHeight + frontWheelRadius) - flCenter.y;
    float frDelta = (frHeight + frontWheelRadius) - frCenter.y;

    // compute average deltaY and move the whole car up/down accordingly
    float deltaY = 0.25f * (blDelta + brDelta + flDelta + frDelta);

    // create translation matrix for height correction
    Matrix4D heightCorrection = Matrix4D::translation({0.0f, deltaY, 0.0f});

    sScene.baseCarTranslationMatrix        = heightCorrection * sScene.baseCarTranslationMatrix;
    sScene.windowCarTranslationMatrix      = heightCorrection * sScene.windowCarTranslationMatrix;
    sScene.bottomLeftWheelTranslationMatrix  = heightCorrection * sScene.bottomLeftWheelTranslationMatrix;
    sScene.bottomRightWheelTranslationMatrix = heightCorrection * sScene.bottomRightWheelTranslationMatrix;
    sScene.topLeftWheelTranslationMatrix     = heightCorrection * sScene.topLeftWheelTranslationMatrix;
    sScene.topRightWheelTranslationMatrix    = heightCorrection * sScene.topRightWheelTranslationMatrix;
    sScene.spareWheelTranslationMatrix       = heightCorrection * sScene.spareWheelTranslationMatrix;

}

void updateCarRotation(const Matrix4D& rotationMatrix)
{
    // extract current car position (pivot) from baseCarTranslationMatrix
    Vector3D pivot = {
        sScene.baseCarTranslationMatrix.n[3][0],
        sScene.baseCarTranslationMatrix.n[3][1],
        sScene.baseCarTranslationMatrix.n[3][2]
    };

    // build the transform to rotate around that pivot
    Matrix4D translateToOrigin = Matrix4D::translation(-pivot);
    Matrix4D translateBack = Matrix4D::translation(pivot);
    Matrix4D fullRotation = translateBack * rotationMatrix * translateToOrigin;

    // rotate all parts around the car’s actual center
    sScene.baseCarTranslationMatrix = fullRotation * sScene.baseCarTranslationMatrix;
    sScene.windowCarTranslationMatrix = fullRotation * sScene.windowCarTranslationMatrix;
    sScene.bottomLeftWheelTranslationMatrix = fullRotation * sScene.bottomLeftWheelTranslationMatrix;
    sScene.bottomRightWheelTranslationMatrix = fullRotation * sScene.bottomRightWheelTranslationMatrix;
    sScene.topLeftWheelTranslationMatrix = fullRotation * sScene.topLeftWheelTranslationMatrix;
    sScene.topRightWheelTranslationMatrix = fullRotation * sScene.topRightWheelTranslationMatrix;
    sScene.spareWheelTranslationMatrix = fullRotation * sScene.spareWheelTranslationMatrix;

    sScene.carTransformationMatrix = rotationMatrix * sScene.carTransformationMatrix;
}

/* function to move and update objects in scene (e.g., move car according to user input) */
void sceneUpdate(float dt) {

    /* constants */
    const float frontWheelRadius = 0.35f;  // Half of 0.7f
    const float rearWheelRadius = 0.5f;    // Half of 1.0f
    const float wheelBase = 2.0f;          // Half of 4.0f
    const float carWidth = 1.0f;           // Half of 2.0f
    const float maxSteeringAngle = to_radians(25.0f);

    /* if 'w' / 's' pressed, car should move forward / backward */
    float forwardMovement = 0.0f;
    if (sInput.buttonPressed[0]) forwardMovement = -1.0f;
    else if (sInput.buttonPressed[1]) forwardMovement = 1.0f;

    /* if 'a' / 'd' pressed, car should steer left / right */
    float steeringDir = 0.0f;
    if (sInput.buttonPressed[2]) steeringDir = -1.0f;
    else if (sInput.buttonPressed[3]) steeringDir = 1.0f;

    if (steeringDir != 0) {
        float steeringAngle = steeringDir * -maxSteeringAngle;
        Matrix4D steeringRot = Matrix4D::rotationY(steeringAngle);
        //sScene.topLeftWheelTransformationMatrix = sScene.carTransformationMatrix * steeringRot;
        //sScene.topRightWheelTransformationMatrix = sScene.carTransformationMatrix * steeringRot;
    }

    /* update forward movement */
    if (forwardMovement != 0) {
        /* direction in local space */
        Vector3D forward = {0.0f, 0.0f, -1.0f};
        Matrix4D carRotation = sScene.carTransformationMatrix;

        /* transform forward vector by car rotation (homogeneous w=0) */
        Vector4D forward4 = {forward.x, forward.y, forward.z, 0.0f};
        Vector4D worldDir4 = carRotation * forward4;
        Vector3D worldDir = {worldDir4.x, worldDir4.y, worldDir4.z};

        /* compute displacement based on velocity and time */
        float distance = sScene.speed * dt;
        Vector3D displacement = worldDir * (distance * forwardMovement);

        /* move car and all attached parts */
        Matrix4D translationMatrix = Matrix4D::translation(displacement);
        updateCarPosition(translationMatrix);

        /* spin all wheels depending on traveled distance */
        float frontAlpha = distance / frontWheelRadius;
        float rearAlpha = distance / rearWheelRadius;

        float newFrontWheelSpin = frontAlpha * -forwardMovement + sScene.frontWheelSpinAccumulator;

        sScene.frontWheelSpinAccumulator = newFrontWheelSpin;

        Matrix4D frontIncrementalSpin = Matrix4D::rotationX(newFrontWheelSpin);
        Matrix4D rearIncrementalSpin = Matrix4D::rotationX(rearAlpha * -forwardMovement );

        /* rear wheels  */
        sScene.bottomLeftWheelTransformationMatrix = sScene.bottomLeftWheelTransformationMatrix * rearIncrementalSpin;
        sScene.bottomRightWheelTransformationMatrix = sScene.bottomRightWheelTransformationMatrix * rearIncrementalSpin;

        float steeringAngleOffset = maxSteeringAngle * steeringDir;

        Matrix4D carRot = sScene.carTransformationMatrix;
        Matrix4D steeringRot = Matrix4D::rotationY(-steeringAngleOffset);

        /* front wheels */
        sScene.topLeftWheelTransformationMatrix = steeringRot * frontIncrementalSpin;
        sScene.topRightWheelTransformationMatrix = steeringRot * frontIncrementalSpin;

        /* if steering, rotate the car’s body around the Y-axis */
        if (steeringDir != 0.0f) {
            float degPerMeter = calculateTurningAnglePerMeter(wheelBase, maxSteeringAngle, carWidth);
            float turnRad = to_radians(degPerMeter * distance);
            Matrix4D carTurn = Matrix4D::rotationY(steeringDir * turnRad * forwardMovement);
            updateCarRotation(carTurn);
        }

    }
}

/* function to draw all objects in the scene */
void sceneDraw() {
    /* clear framebuffer color */
    glClearColor(135.0f / 255, 206.0f / 255, 235.0f / 255, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*------------ render scene -------------*/
    glUseProgram(sScene.shaderColor.id);
    shaderUniform(sScene.shaderColor, "uProj", cameraProjection(sScene.camera));
    shaderUniform(sScene.shaderColor, "uView", cameraView(sScene.camera));

    /* draw ground */
    shaderUniform(sScene.shaderColor, "uModel", Matrix4D::identity());
    glBindVertexArray(sScene.ground.mesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.ground.mesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    /* ---------- cubes ---------- */

    /* base car */
    shaderUniform(sScene.shaderColor, "uModel",
        sScene.baseCarTranslationMatrix *
        sScene.baseCarTransformationMatrix *
        sScene.baseCarScalingMatrix);
    glBindVertexArray(sScene.baseCarMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.baseCarMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    /* window car */
    shaderUniform(sScene.shaderColor, "uModel",
        sScene.windowCarTranslationMatrix *
        sScene.windowCarTransformationMatrix *
        sScene.windowCarScalingMatrix);
    glBindVertexArray(sScene.windowCarMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.windowCarMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    /* ---------- cylinders ---------- */

    /* bottom left wheel */
    shaderUniform(sScene.shaderColor, "uModel",
        sScene.bottomLeftWheelTranslationMatrix *
        sScene.bottomLeftWheelTransformationMatrix *
        sScene.bottomLeftWheelScalingMatrix);
    glBindVertexArray(sScene.bottomLeftWheelMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.bottomLeftWheelMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    /* bottom right wheel */
    shaderUniform(sScene.shaderColor, "uModel",
        sScene.bottomRightWheelTranslationMatrix *
        sScene.bottomRightWheelTransformationMatrix *
        sScene.bottomRightWheelScalingMatrix);
    glBindVertexArray(sScene.bottomRightWheelMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.bottomRightWheelMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    /* top left wheel */
    shaderUniform(sScene.shaderColor, "uModel",
        sScene.topLeftWheelTranslationMatrix *
        sScene.topLeftWheelTransformationMatrix *
        sScene.topLeftWheelScalingMatrix);
    glBindVertexArray(sScene.topLeftWheelMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.topLeftWheelMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    /* top right wheel */
    shaderUniform(sScene.shaderColor, "uModel",
        sScene.topRightWheelTranslationMatrix *
        sScene.topRightWheelTransformationMatrix *
        sScene.topRightWheelScalingMatrix);
    glBindVertexArray(sScene.topRightWheelMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.topRightWheelMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    /* spare wheel */
    shaderUniform(sScene.shaderColor, "uModel",
        sScene.spareWheelTranslationMatrix *
        sScene.spareWheelTransformationMatrix *
        sScene.spareWheelScalingMatrix);
    glBindVertexArray(sScene.spareWheelMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.spareWheelMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    glCheckError();

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);
}

int main(int argc, char **argv) {
    /* create window/context */
    int width = 1280;
    int height = 720;
    GLFWwindow *window = windowCreate("Assignment 3 - Transformations, User Input and Camera", width, height);
    if (!window) {
        return EXIT_FAILURE;
    }

    /* set window callbacks */
    glfwSetKeyCallback(window, callbackKey);
    glfwSetCursorPosCallback(window, callbackMousePos);
    glfwSetMouseButtonCallback(window, callbackMouseButton);
    glfwSetScrollCallback(window, callbackMouseScroll);
    glfwSetFramebufferSizeCallback(window, callbackWindowResize);

    /*---------- init opengl stuff ------------*/
    glEnable(GL_DEPTH_TEST);

    /* setup scene */
    sceneInit(width, height);

    /*-------------- main loop ----------------*/
    double timeStamp = glfwGetTime();
    double timeStampNew = 0.0;

    /* loop until user closes window */
    while (!glfwWindowShouldClose(window)) {
        /* poll and process input and window events */
        glfwPollEvents();

        /* update camera and model matrices */
        timeStampNew = glfwGetTime();
        sceneUpdate(timeStampNew - timeStamp);
        timeStamp = timeStampNew;

        /* draw all objects in the scene */
        sceneDraw();

        /* swap front and back buffer */
        glfwSwapBuffers(window);
    }

    /*-------- cleanup --------*/
    /* delete opengl shader and buffers */
    shaderDelete(sScene.shaderColor);
    groundDelete(sScene.ground);

    /* cleanup glfw/glcontext */
    windowDelete(window);

    return EXIT_SUCCESS;
}
