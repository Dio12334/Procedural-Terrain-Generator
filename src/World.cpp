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
		/* float offset_x  = 5.9f; */
		/* float offset_y  = 5.1f; */
		/* float offset_z  = 0.05f; */
		/* float lacunarity    = 1.99f; */
		/* float persistance   = 0.5f; */
		

		/* const SimplexNoise simplex(0.1/scale, 0.5f, lacunarity, persistance); */
		const SimplexNoise simplex(1/400.f);
		const int octaves = static_cast<int>(5 + std::log(scale));
		
		const float radius = m_WIDTH/(M_PI*2.f);

		for(unsigned j = 0; j < m_HEIGHT; ++j){
			/* const float Y = static_cast<float>(j - m_HEIGHT/2.f + offset_y*scale); */
			for(unsigned i = 0; i < m_WIDTH; ++i){
				/* const float X = static_cast<float>(i - m_WIDTH/2.f + offset_x*scale); */
				
				float angle = i/radius;

				float x = std::sin(angle)*radius;
				float y = j;
				float z = std::cos(angle)*radius;
				
				/* std::cout << M_2_PI << '\n'; */
				/* std::cout << x << " " << y << " " << z << '\n'; */
				const float noise = simplex.fractal(octaves, x + seed, y , z + seed);
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
	
		auto TemperatureBasedOnLatitude = [](const float& latitude){
			if(latitude < 1.f/6.f)
				return 0.74;
			else{
				return -0.96*((latitude - 1.f/6.f)*(latitude - 1.f/6.f)) + 0.74;
				}
			};

		for(unsigned j = 0; j < m_HEIGHT; ++j){
			auto distance_from_equator = DistanceFromEquatorNormalized(j);

			for(unsigned i = 0; i < m_WIDTH; ++i){
				const auto elevation = m_elevation_map[j][i];
				float elevation_above_sealevel_in_km = elevation*elevation_scale;
				float elevation_based_temperature_decline = percentage_temperature_decline_per_km * elevation_above_sealevel_in_km;
				

				float temperature = TemperatureBasedOnLatitude(distance_from_equator);

				temperature = (temperature - 0.5f)*2.f;
				temperature -= elevation > 0? elevation_based_temperature_decline : 0.1f;
				temperature = std::clamp(temperature, -1.f, 1.f);
				m_temperature_map[j][i] = temperature;

			}	
		}

		auto surface = psx::img::CreateBlankSurface(m_WIDTH, m_HEIGHT);
		/* float maximum_temperature = +45.f, minimum_temperature = -25.f; */
		for(unsigned j = 0; j < m_HEIGHT; ++j){
			for(unsigned i = 0; i < m_WIDTH; ++i){
				auto grey = (m_temperature_map[j][i] +1.f)/(2.f);
				psx::img::Color greyScale(grey, grey, grey);
				psx::img::SetPixelOnSurface(surface, i, j, greyScale.ToUint32());
			}
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
		return std::abs(delta)/range;
	}
}
