#ifndef WORLD_H
#define WORLD_H

#include <array>

namespace sim {

    enum Biome {

    };

    class World{
        public:
            using Map = std::array<std::array<float, 812>, 406>;
			using SeedType = float;
            
			void GenerateNewMap();
			SeedType getSeed();

        private:

			SeedType seed;

            Map m_elevation_map;
            Map m_temperature_map;
            Map m_circulation_map;
            Map m_evapotranspiration_map;
            Map m_humidity_map;
            Map m_precipitation_map;

			void GenerateSeed();

            void GenerateElevation();
            void GenerateTemperature();
            void GenerateCirculation();
            void GenerateEvapotranspiration();
            void GenerateHumidity();
            void GeneratePrecipitation();

            float DistanceFromEquator(float y);
            float DistanceFromEquatorNormalized(float y);
    };
}
#endif

