/**
 * @file Shader.h
 * @brief Định nghĩa lớp Shader nạp mã nguồn GLSL từ tệp, biên dịch và truyền tham số Uniform vào GPU.
 */

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void use() const;

    // === TRUYỀN DỮ LIỆU UNIFORM XUỐNG SHADER ===
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    void checkCompileErrors(unsigned int shader, const std::string& type);
};

#endif // SHADER_H
