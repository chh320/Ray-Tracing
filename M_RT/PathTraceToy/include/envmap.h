#ifndef ENVMAP_H
#define ENVMAP_H

class Envmap {
public:
	Envmap(const std::string& evnmapPath) {
        //stbi_set_flip_vertically_on_load(true);
        data = stbi_loadf(evnmapPath.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glGenTextures(1, &envMapTex);
            glBindTexture(GL_TEXTURE_2D, envMapTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            std::cout << "Successed to load HDR image." << std::endl;
        }
        else
        {
            std::cout << "Failed to load HDR image." << std::endl;
        }
	}

    void generateHdrCache();
    float* calculateHdrCache(float* HDR, int width, int height);

	GLuint envMapTex;
    GLuint envMapCache;
    float* data;
    int width, height, nrComponents;
};


void Envmap::generateHdrCache() {
    float* cache = calculateHdrCache(data, width, height);
    if (cache)
    {
        glGenTextures(1, &envMapCache);
        glBindTexture(GL_TEXTURE_2D, envMapCache);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, cache);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(cache);
        stbi_image_free(data);
        std::cout << "Successed to load HDR cache." << std::endl;
    }
    else
    {
        std::cout << "Failed to load HDR cache." << std::endl;
    }
}

float* Envmap::calculateHdrCache(float* HDR, int width, int height) {
    float lumSum = 0.0;

    // initialize w * h pdf and sum the light of hdr
    std::vector<std::vector<float>> pdf(height);
    for (auto& line : pdf) line.resize(width);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float R = HDR[3 * (i * width + j)];
            float G = HDR[3 * (i * width + j) + 1];
            float B = HDR[3 * (i * width + j) + 2];
            float lum = 0.2 * R + 0.7 * G + 0.1 * B;
            pdf[i][j] = lum;
            lumSum += lum;
        }
    }

    // normalize
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            pdf[i][j] /= lumSum;
        }
    }

    // accumulate each column to get the edge-pdf of x
    std::vector<float> pdf_x_margin;
    pdf_x_margin.resize(width);
    for (int j = 0; j < width; j++) {
        for (int i = 0; i < height; i++) {
            pdf_x_margin[j] += pdf[i][j];
        }
    }

    // calculate edge-cdf of x
    std::vector<float> cdf_x_margin = pdf_x_margin;
    for (int i = 1; i < width; i++) {
        cdf_x_margin[i] += cdf_x_margin[i - 1];
    }

    // calculate conditional-pdf of y with X = x
    std::vector<std::vector<float>> pdf_y_conditional = pdf;
    for (int j = 0; j < width; j++)
        for (int i = 0; i < height; i++)
            pdf_y_conditional[i][j] /= pdf_x_margin[j];

    // calculate conditional-cdf of y with X = x
    std::vector<std::vector<float >> cdf_y_conditional = pdf_y_conditional;
    for (int j = 0; j < width; j++)
        for (int i = 1; i < height; i++)
            cdf_y_conditional[i][j] += cdf_y_conditional[i - 1][j];

    // transpose
    // cdf_y_condiciton[i] means conditional-cdf of y with X = i
    std::vector<std::vector<float>> temp = cdf_y_conditional;
    cdf_y_conditional = std::vector<std::vector<float>>(width);
    for (auto& line : cdf_y_conditional) line.resize(height);
    for (int j = 0; j < width; j++)
        for (int i = 0; i < height; i++)
            cdf_y_conditional[j][i] = temp[i][j];

    std::vector<std::vector<float>> sample_x(height);
    for (auto& line : sample_x) line.resize(width);
    std::vector<std::vector<float>> sample_y(height);
    for (auto& line : sample_y) line.resize(width);
    std::vector<std::vector<float>> sample_p(height);
    for (auto& line : sample_p) line.resize(width);
    for (int j = 0; j < width; j++) {
        for (int i = 0; i < height; i++) {
            float xi_1 = float(i) / height;
            float xi_2 = float(j) / width;

            // 用 xi_1 在 cdf_x_margin 中 lower bound 得到样本 x
            int x = std::lower_bound(cdf_x_margin.begin(), cdf_x_margin.end(), xi_1) - cdf_x_margin.begin();
            // 用 xi_2 在 X=x 的情况下得到样本 y
            int y = std::lower_bound(cdf_y_conditional[x].begin(), cdf_y_conditional[x].end(), xi_2) - cdf_y_conditional[x].begin();

            // 存储纹理坐标 xy 和 xy 位置对应的概率密度
            sample_x[i][j] = float(float(x) / width);
            sample_y[i][j] = float(float(y) / height);
            sample_p[i][j] = pdf[i][j];
        }
    }

    float* cache = new float[width * height * 3];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            cache[3 * (i * width + j)] = sample_x[i][j];        // R
            cache[3 * (i * width + j) + 1] = sample_y[i][j];    // G
            cache[3 * (i * width + j) + 2] = sample_p[i][j];    // B
        }
    }

    return cache;
}

#endif // !ENVMAP_H
