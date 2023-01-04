#include "World.h"
#include "../Psx/img.h"
#include "../SimplexNoise/src/SimplexNoise.h"
#include <cmath>
#include <limits>
#include <random>
#include <iostream>
#include <algorithm>

namespace sim {
	
	void World::GenerateNewMap(){
		GenerateSeed();
		GenerateElevation();
		GenerateTemperature();
	}

	World::SeedType World::getSeed(){
		return seed;
	}

	void World::GenerateSeed(){
		std::random_device rd;
		seed = std::uniform_real_distribution<SeedType>(-10000, 10000)(rd);
	}

	void World::GenerateElevation(){
		float scale     = 40.f;
		float offset_x  = 5.9f;
		float offset_y  = 5.1f;
		float offset_z  = 0.05f;
		float lacunarity    = 1.99f;
		float persistance   = 0.5f;
		

		const SimplexNoise simplex(0.1/scale, 0.5f, lacunarity, persistance);
		const int octaves = static_cast<int>(5 + std::log(scale));


		for(unsigned j = 0; j < m_HEIGHT; ++j){
			const float y = static_cast<float>(j - m_HEIGHT/2.f + offset_y*scale);
			for(unsigned i = 0; i < m_WIDTH; ++i){
				const float x = static_cast<float>(i - m_WIDTH/2.f + offset_x*scale);
				
				const float noise = simplex.fractal(octaves, x + seed, y + seed) + offset_z;
				m_elevation_map[j][i] = noise;
			}
		}

		auto surface = psx::img::CreateBlankSurface(m_WIDTH, m_HEIGHT);

		for(unsigned j = 0; j < m_HEIGHT; ++j){
			for(unsigned i = 0; i < m_WIDTH; ++i){
				auto grey = (m_elevation_map[j][i] + 1.f)/2.f;
				psx::img::Color greyScale(grey,grey,grey);
				if(grey < 0.5f){
					greyScale.r = greyScale.g = 0.f;
				}
				psx::img::SetPixelOnSurface(surface, i, j, greyScale.ToUint32());
			}
		}

		psx::img::SaveSurfaceAsPNG(surface, "Elevation");
		
	}

	void World::GenerateTemperature(){


		const auto percentage_temperature_decline_per_km = 10.f/70.f;
		const auto elevation_scale = 10.f;
	
		float mint = std::numeric_limits<float>::max();
		float maxt = std::numeric_limits<float>::lowest();

		for(unsigned j = 0; j < m_HEIGHT; ++j){
			auto distance_from_equator = DistanceFromEquatorNormalized(j);
			auto distance_from_equator_squared = distance_from_equator * distance_from_equator;
			for(unsigned i = 0; i < m_WIDTH; ++i){
				const auto elevation = m_elevation_map[j][i];
				float elevation_above_sealevel_in_km = elevation*elevation_scale;
				float elevation_based_temperature_decline = percentage_temperature_decline_per_km * elevation_above_sealevel_in_km;
				float temperature = -0.67*distance_from_equator_squared + 0.74;
				temperature = (temperature - 0.5f)*2.f;
				temperature -= elevation > 0? elevation_based_temperature_decline : 0.1f;
				temperature = std::clamp(temperature, -1.f, 1.f);
				m_temperature_map[j][i] = temperature;

				if(temperature > maxt){
					maxt = temperature;
				}
				if(temperature < mint){
					mint = temperature;
				}
			}	
		}

		std::cout << "Temperatures max min: " << maxt << " " << mint << std::endl;
		auto surface = psx::img::CreateBlankSurface(m_WIDTH, m_HEIGHT);
		float maximum_temperature = +45.f, minimum_temperature = -25.f;
		float temperature_range = maximum_temperature - minimum_temperature;
		for(unsigned j = 0; j < m_HEIGHT; ++j){
			for(unsigned i = 0; i < m_WIDTH; ++i){
				auto grey = (m_temperature_map[j][i] +1.f)/(2.f);
				psx::img::Color greyScale(grey, grey, grey);
				std::cout << grey *temperature_range + minimum_temperature << " ";
				psx::img::SetPixelOnSurface(surface, i, j, greyScale.ToUint32());
			}
			std::cout << '\n';
		}

		psx::img::SaveSurfaceAsPNG(surface, "Temperature");
	}

	void World::GenerateCirculation(){

	}
	void World::GenerateEvapotranspiration(){

	}
	void World::GenerateHumidity(){

	}
	void World::GeneratePrecipitation(){

	}

	float World::DistanceFromEquator(float y){
		return std::abs(y - m_HEIGHT/2.f);
	}
	float World::DistanceFromEquatorNormalized(float y){
		const float LATITUDE = (y/m_HEIGHT - 0.5f)*2.f;
		const float EQUATOR = 0.f;
		float delta = LATITUDE - EQUATOR;
		auto sign = [](const float& val) -> float {
			if(val < 0.f){
				return -1.f;
			}
			else if(val == 0.f){
				return 0.f;
			}
			else{
				return 1.0f;
			}
		};

		float range  = 1.f - sign(delta) * EQUATOR;
		auto first_val = std::abs(delta)/range;
		auto second_val = DistanceFromEquator(y)/(m_HEIGHT/2.f);
		std::cout << first_val << " " << second_val << '\n';
		return first_val;
	}
}
