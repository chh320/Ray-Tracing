#include <time.h>
#include <math.h>
#include <string>

#define STB_IAMGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_write.h>

#include <Scene.h>

using namespace std;

int main(int argc, char* argv[]) {
	srand((unsigned int)time(0));

	std::string sceneFile;

	for (int i = 1; i < argc; i++) {
		const std::string arg(argv[i]);
		if (arg == "-s" || arg == "-scene") {
			sceneFile = argv[++i];
		}
		else if (arg[0] == '-') {
			printf("Unknown option %s \n", arg.c_str());
			exit(0);
		}
	}
	if (!sceneFile.empty()) {
		scene = new Scene();
	}
}