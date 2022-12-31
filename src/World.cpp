#include "World.h"
#include "../Psx/img.h"
#include "../SimplexNoise/src/SimplexNoise.h"
#include <cmath>
#include <limits>

namespace sim {
	
	void World::GenerateNewMap(){
		GenerateElevation();
		GenerateTemperature();
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

		const auto HEIGHT = m_elevation_map.size();
		const auto WIDTH = m_elevation_map[0].size();

		for(unsigned j = 0; j < HEIGHT; ++j){
			const float y = static_cast<float>(j - HEIGHT/2.f + offset_y*scale);
			for(unsigned i = 0; i < WIDTH; ++i){
				const float x = static_cast<float>(i - WIDTH/2.f + offset_x*scale);
				
				const float noise = simplex.fractal(octaves, x, y) + offset_z;
				m_elevation_map[j][i] = noise;
			}
		}

		auto surface = psx::img::CreateBlankSurface(WIDTH, HEIGHT);

		for(unsigned j = 0; j < HEIGHT; ++j){
			for(unsigned i = 0; i < WIDTH; ++i){
				auto grey = (m_elevation_map[j][i] + 1.f)/2.f;
				psx::img::Color greyScale(grey,grey,grey);
				psx::img::SetPixelOnSurface(surface, i, j, greyScale.ToUint32());
			}
		}

		psx::img::SaveSurfaceAsPNG(surface, "Elevation");
		
	}

	void World::GenerateTemperature(){
		const auto HEIGHT = m_temperature_map.size();
		const auto WIDTH = m_temperature_map[0].size();

		const auto percentage_temperature_decline_per_km = 10.f/70.f;
		const auto elevation_scale = 10.f;
	
		float mint = std::numeric_limits<float>::max();
		float maxt = std::numeric_limits<float>::lowest();
		for(unsigned j = 0; j < HEIGHT; ++j){
			auto distance_from_equator = DistanceFromEquatorNormalized(j);
			auto distance_from_equator_squared = distance_from_equator * distance_from_equator;
			for(unsigned i = 0; i < WIDTH; ++i){
				const auto elevation = m_elevation_map[j][i];
				float elevation_above_sealevel_in_km = elevation*elevation_scale;
				float elevation_based_temperature_decline = percentage_temperature_decline_per_km * elevation_above_sealevel_in_km;
				float temperature = 1.f - distance_from_equator_squared;
				temperature -= elevation > 0? elevation_based_temperature_decline : 0.1f;
				m_temperature_map[j][i] = temperature;

				if(temperature > maxt){
					maxt = temperature;
				}
				if(temperature < mint){
					mint = temperature;
				}
			}	
		}

		auto surface = psx::img::CreateBlankSurface(WIDTH, HEIGHT);

		for(unsigned j = 0; j < HEIGHT; ++j){
			for(unsigned i = 0; i < WIDTH; ++i){
				auto grey = (m_temperature_map[j][i] - mint)/(maxt - mint);
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
		const float HEIGHT = m_elevation_map.size();
		return std::abs(y - HEIGHT/2);
	}
	float World::DistanceFromEquatorNormalized(float y){
	
		const float HEIGHT = m_elevation_map.size();
		return DistanceFromEquator(y)/(HEIGHT/2);
	}
}
