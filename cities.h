#ifndef CITIES_H
#define CITIES_H

typedef struct {
    const char *name;
    const char *country;
    float lat;
    float lon;
} City;

/* Major world metros, weighted toward Indian cities as requested. */
static const City CITIES[] = {
    /* India */
    {"Mumbai", "India", 19.0760f, 72.8777f},
    {"Delhi", "India", 28.6139f, 77.2090f},
    {"Bangalore", "India", 12.9716f, 77.5946f},
    {"Hyderabad", "India", 17.3850f, 78.4867f},
    {"Ahmedabad", "India", 23.0225f, 72.5714f},
    {"Chennai", "India", 13.0827f, 80.2707f},
    {"Kolkata", "India", 22.5726f, 88.3639f},
    {"Pune", "India", 18.5204f, 73.8567f},
    {"Jaipur", "India", 26.9124f, 75.7873f},
    {"Surat", "India", 21.1702f, 72.8311f},
    {"Lucknow", "India", 26.8467f, 80.9462f},
    {"Kanpur", "India", 26.4499f, 80.3319f},
    {"Nagpur", "India", 21.1458f, 79.0882f},
    {"Indore", "India", 22.7196f, 75.8577f},
    {"Thane", "India", 19.2183f, 72.9781f},
    {"Bhopal", "India", 23.2599f, 77.4126f},
    {"Visakhapatnam", "India", 17.6868f, 83.2185f},
    {"Patna", "India", 25.5941f, 85.1376f},
    {"Vadodara", "India", 22.3072f, 73.1812f},
    {"Ghaziabad", "India", 28.6692f, 77.4538f},
    {"Ludhiana", "India", 30.9010f, 75.8573f},
    {"Agra", "India", 27.1767f, 78.0081f},
    {"Nashik", "India", 19.9975f, 73.7898f},
    {"Faridabad", "India", 28.4089f, 77.3178f},
    {"Meerut", "India", 28.9845f, 77.7064f},
    {"Rajkot", "India", 22.3039f, 70.8022f},
    {"Varanasi", "India", 25.3176f, 82.9739f},
    {"Srinagar", "India", 34.0837f, 74.7973f},
    {"Aurangabad", "India", 19.8762f, 75.3433f},
    {"Dhanbad", "India", 23.7957f, 86.4304f},
    {"Amritsar", "India", 31.6340f, 74.8723f},
    {"Navi Mumbai", "India", 19.0330f, 73.0297f},
    {"Allahabad", "India", 25.4358f, 81.8463f},
    {"Howrah", "India", 22.5958f, 88.2636f},
    {"Jabalpur", "India", 23.1815f, 79.9864f},
    {"Gwalior", "India", 26.2183f, 78.1828f},
    {"Jodhpur", "India", 26.2389f, 73.0243f},
    {"Coimbatore", "India", 11.0168f, 76.9558f},
    {"Vijayawada", "India", 16.5062f, 80.6480f},
    {"Mysore", "India", 12.2958f, 76.6394f},
    {"Bhubaneswar", "India", 20.2961f, 85.8245f},
    {"Salem", "India", 11.6643f, 78.1460f},
    {"Guwahati", "India", 26.1445f, 91.7362f},
    {"Chandigarh", "India", 30.7333f, 76.7794f},

    /* Rest of world */
    {"London", "United Kingdom", 51.5074f, -0.1278f},
    {"Paris", "France", 48.8566f, 2.3522f},
    {"Berlin", "Germany", 52.5200f, 13.4050f},
    {"New York", "United States", 40.7128f, -74.0060f},
    {"Tokyo", "Japan", 35.6762f, 139.6503f},
    {"Los Angeles", "United States", 34.0522f, -118.2437f},
    {"Chicago", "United States", 41.8781f, -87.6298f},
    {"Toronto", "Canada", 43.6532f, -79.3832f},
    {"Sydney", "Australia", -33.8688f, 151.2093f},
    {"Melbourne", "Australia", -37.8136f, 144.9631f},
    {"Dubai", "United Arab Emirates", 25.2048f, 55.2708f},
    {"Istanbul", "Turkey", 41.0082f, 28.9784f},
    {"Moscow", "Russia", 55.7558f, 37.6173f},
    {"Beijing", "China", 39.9042f, 116.4074f},
    {"Shanghai", "China", 31.2304f, 121.4737f},
    {"Singapore", "Singapore", 1.3521f, 103.8198f},
    {"Hong Kong", "Hong Kong", 22.3193f, 114.1694f},
    {"Seoul", "South Korea", 37.5665f, 126.9780f},
    {"Bangkok", "Thailand", 13.7563f, 100.5018f},
    {"Jakarta", "Indonesia", -6.2088f, 106.8456f},
    {"Cairo", "Egypt", 30.0444f, 31.2357f},
    {"Johannesburg", "South Africa", -26.2041f, 28.0473f},
    {"Lagos", "Nigeria", 6.5244f, 3.3792f},
    {"Nairobi", "Kenya", -1.2921f, 36.8219f},
    {"Mexico City", "Mexico", 19.4326f, -99.1332f},
    {"Sao Paulo", "Brazil", -23.5505f, -46.6333f},
    {"Buenos Aires", "Argentina", -34.6037f, -58.3816f},
    {"Madrid", "Spain", 40.4168f, -3.7038f},
    {"Rome", "Italy", 41.9028f, 12.4964f},
    {"Vienna", "Austria", 48.2082f, 16.3738f},
    {"Amsterdam", "Netherlands", 52.3676f, 4.9041f},
    {"Brussels", "Belgium", 50.8503f, 4.3517f},
    {"Stockholm", "Sweden", 59.3293f, 18.0686f},
    {"Oslo", "Norway", 59.9139f, 10.7522f},
    {"Copenhagen", "Denmark", 55.6761f, 12.5683f},
    {"Helsinki", "Finland", 60.1699f, 24.9384f},
    {"Warsaw", "Poland", 52.2297f, 21.0122f},
    {"Prague", "Czech Republic", 50.0755f, 14.4378f},
    {"Budapest", "Hungary", 47.4979f, 19.0402f},
    {"Athens", "Greece", 37.9838f, 23.7275f},
    {"Lisbon", "Portugal", 38.7223f, -9.1393f},
    {"Dublin", "Ireland", 53.3498f, -6.2603f},
    {"Reykjavik", "Iceland", 64.1466f, -21.9426f},
    {"Cape Town", "South Africa", -33.9249f, 18.4241f},
    {"Tel Aviv", "Israel", 32.0853f, 34.7818f},
    {"Riyadh", "Saudi Arabia", 24.7136f, 46.6753f},
    {"Tehran", "Iran", 35.6892f, 51.3890f},
};

static inline int cities_count(void) {
    return (int)(sizeof(CITIES) / sizeof(CITIES[0]));
}

static inline const City *cities_list(void) {
    return CITIES;
}

#endif
