#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "General.h"
#define COMPRESS
#define AIRLINE_COMPRESS_FILE_NAME "airline_compress.bin"




const char* str[eNofOptions] = { "Add Flight", "Add Airport",
								"Print Airline", "Print all Airports",
								"Sort Flights", "Search Flight",
								"Print number of flights from origin airport name",
								"Print all flights with plane code",
								"Print all flights with plane type" };

int main()
{
	AirportManager	manager;
	Airline			company;

	initManagerAndAirline(&manager, &company, AIRLINE_FILE_NAME);

	int option;
	int stop = 0;
	int compress = 0 , binary = 0;
	char* airlineFileName = "airline_compress.bin";
	do
	{
		option = menu();
		switch (option)
		{
		case eAddFlight:
			if (!addFlight(&company, &manager))
				printf("Error adding flight\n");
			break;


		case eAddAirport:
			if (!addAirport(&manager))
				printf("Error adding airport\n");
			break;

		case ePrintCompany:
			printCompany(&company);
			break;

		case ePrintAirports:
			printAirports(&manager);
			break;

		case eSortFlights:
			sortFlight(&company);
			break;

		case eSearchFlight:
			findFlight(&company);
			break;

		case ePrintNumFlightsOrig:
			doCountFlightsFromName(&company);
			break;

		case ePrintFlightsPlaneCode:
			doPrintFlightsWithPlaneCode(&company);
			break;

		case ePrintFlightsPlaneType:
			doPrintFlightsWithPlaneType(&company);
			break;

		case EXIT:
			printf("Bye bye\n");
			stop = 1;
			break;

		default:
			printf("Wrong option\n");
			break;
		}
	} while (!stop);

	saveManagerToFile(&manager, MANAGER_FILE_NAME);

#ifdef COMPRESS

	airlineFileName = AIRLINE_COMPRESS_FILE_NAME;
	compress = 1;
	if (!writeAirlineToCompressBinaryFile(&company))
	{
		printf("The compress file binary writing failed\n");
	}
#else
	saveAirlineToFile(&company, AIRLINE_FILE_NAME);
#endif
	


	freeManager(&manager);
	freeCompany(&company);

	system("pause");
	return 1;
}

int initManagerAndAirline(AirportManager* pManager, Airline* pCompany,const char* ailineFileName)
{
	puts("==========================");
	puts("Geting data from files...");
	int res = initManager(pManager, MANAGER_FILE_NAME);
	if (!res)
	{
		printf("error init manager\n");
		return 0;
	}
	
#ifdef COMPRESS
	initAirlineFromCompressFile(pCompany, AIRLINE_COMPRESS_FILE_NAME);
	if (!initAirlineFromCompressFile)
		initAirline(pCompany);

#else 

initAirlineFromFile(pCompany, pManager, ailineFileName);
if(!initAirlineFromFile)
initAirline(pCompany);
return 1;

#endif
	

puts("==========================");
	return 1;



}

int menu()
{
	int option;
	printf("\n\n");
	printf("Please choose one of the following options\n");
	for(int i = 0 ; i < eNofOptions ; i++)
		printf("%d - %s\n",i,str[i]);
	printf("%d - Quit\n", EXIT);
	scanf("%d", &option);
	//clean buffer
	char tav;
	scanf("%c", &tav);
	return option;
}

