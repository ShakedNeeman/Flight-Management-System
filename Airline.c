#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Airline.h"
#include "Airport.h"
#include "General.h"
#include "fileHelper.h"
#define MIN_YEAR 2021
#define MIN_CHAR 65


static const char* sortOptStr[eNofSortOpt] = {
	"None","Source Name", "Dest Name" , "Date", "Plane Code" };


int		initAirlineFromFile(Airline* pComp, AirportManager* pManaer, const char* fileName)
{
	L_init(&pComp->flighDateList);
	if (loadAirlineFromFile(pComp, pManaer, fileName))
		return initDateList(pComp);

	return 0;
}

int		initAirlineFromCompressFile(Airline* pComp, const char* fileName)
{
	L_init(&pComp->flighDateList);
	if (readAirlineToCompressBinaryFile(pComp, fileName))
		return initDateList(pComp);

	return 0;
}


void	initAirline(Airline* pComp)
{
	printf("-----------  Init Airline\n");
	pComp->name = getStrExactName("Enter Airline name");
	pComp->flightArr = NULL;
	pComp->flightCount = 0;
	L_init(&pComp->flighDateList);
}

int	addFlight(Airline* pComp,const AirportManager* pManager)
{
	if (pManager->airportsCount < 2)
	{
		printf("There are not enough airport to set a flight\n");
		return 0;
	}
	pComp->flightArr = (Flight**)realloc(pComp->flightArr, (pComp->flightCount+1) * sizeof(Flight*));
	if (!pComp->flightArr)
		return 0;
	pComp->flightArr[pComp->flightCount] = (Flight*)calloc(1,sizeof(Flight));
	if (!pComp->flightArr[pComp->flightCount])
		return 0;
	initFlight(pComp->flightArr[pComp->flightCount],pManager);

	int res = insertFlightDateToList(&pComp->flighDateList, pComp->flightArr[pComp->flightCount]);
	
	pComp->flightCount++;
	pComp->sortOpt = eNone; //new fight not sorted!
	return res;
}

void printCompany(const Airline* pComp)
{
	printf("Airline %s\n", pComp->name);
	printf("Has %d flights\n",pComp->flightCount);
	generalArrayFunction((void*)pComp->flightArr, pComp->flightCount, sizeof(Flight*), printFlightV);
	printf("\nFlight Date List:");
	L_print(&pComp->flighDateList, printDate);
}

void	doCountFlightsFromName(const Airline* pComp)
{
	if (pComp->flightCount == 0)
	{
		printf("No flight in company\n");
		return;
	}

	char* tempName = getStrExactName("Please enter origin airport name");

	int count = 0;
	for (int i = 0; i < pComp->flightCount; i++)
	{
		if (isFlightFromSourceName(pComp->flightArr[i], tempName))
			count++;
	}

	free(tempName);

	if (count != 0)
		printf("There are %d ", count);
	else
		printf("There are No ");

	printf("flights from this airport\n");
}

void	doPrintFlightsWithPlaneCode(const Airline* pComp)
{
	char code[MAX_STR_LEN];
	getPlaneCode(code);
	printf("All flights with plane code %s:\n",code);
	for (int i = 0; i < pComp->flightCount; i++)
	{
		if (isPlaneCodeInFlight(pComp->flightArr[i], code))
			printFlight(pComp->flightArr[i]);
	}
	printf("\n");

}

void	doPrintFlightsWithPlaneType(const Airline* pComp)
{
	ePlaneType type = getPlaneType();

	printf("All flights with plane type %s:\n", GetPlaneTypeStr(type));
	for (int i = 0; i < pComp->flightCount; i++)
	{
		if (isPlaneTypeInFlight(pComp->flightArr[i], type))
			printFlight(pComp->flightArr[i]);
	}
	printf("\n");
}

int		saveAirlineToFile(const Airline* pComp, const char* fileName)
{
	FILE* fp;
	fp = fopen(fileName, "wb");
	if (!fp) {
		printf("Error open copmpany file to write\n");
		return 0;
	}

	if (!writeStringToFile(pComp->name, fp, "Error write comapny name\n"))
		return 0;

	if (!writeIntToFile((int)pComp->sortOpt, fp, "Error write sort option\n"))
		return 0;

	if (!writeIntToFile(pComp->flightCount, fp, "Error write flight count\n"))
		return 0;

	for (int i = 0; i < pComp->flightCount; i++)
	{
		if (!saveFlightToFile(pComp->flightArr[i], fp))
			return 0;
	}

	fclose(fp);
	return 1;
}

int		loadAirlineFromFile(Airline* pComp, const AirportManager* pManager, 
	const char* fileName)
{
	FILE* fp;
	fp = fopen(fileName, "rb");
	if (!fp)
	{
		printf("Error open company file\n");
		return 0;
	}

	pComp->flightArr = NULL;


	pComp->name = readStringFromFile(fp, "Error reading company name\n");
	if (!pComp->name)
		return 0;

	int opt;
	if (!readIntFromFile(&opt, fp, "Error reading sort option\n"))
		return 0;

	pComp->sortOpt = (eSortOption)opt;

	if (!readIntFromFile(&pComp->flightCount, fp, "Error reading flight count name\n"))
		return 0;
	
	if (pComp->flightCount > 0)
	{
		pComp->flightArr = (Flight**)malloc(pComp->flightCount * sizeof(Flight*));
		if (!pComp->flightArr)
		{
			printf("Alocation error\n");
			fclose(fp);
			return 0;
		}
	}
	else
		pComp->flightArr = NULL;

	for (int i = 0; i < pComp->flightCount; i++)
	{
		pComp->flightArr[i] = (Flight*)calloc(1, sizeof(Flight));
		if (!pComp->flightArr[i])
		{
			printf("Alocation error\n");
			free(pComp->flightArr);
			fclose(fp);
			return 0;
		}
		if (!loadFlightFromFile(pComp->flightArr[i], pManager, fp))
		{
			free(pComp->flightArr);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);
	return 1;
}

void	sortFlight(Airline* pComp)
{
	pComp->sortOpt = showSortMenu();
	int(*compare)(const void* air1, const void* air2) = NULL;

	switch (pComp->sortOpt)
	{
	case eSourceName:
		compare = compareFlightBySourceName;
		break;
	case eDestName:
		compare = compareFlightByDestName;
		break;
	case eDate:
		compare = compareFlightByDate;
		break;
	case ePlaneCode:
		compare = compareFlightByPlaneCode;
		break;

	}

	if (compare != NULL)
		qsort(pComp->flightArr, pComp->flightCount, sizeof(Flight*), compare);
}

void	findFlight(const Airline* pComp)
{
	int(*compare)(const void* air1, const void* air2) = NULL;
	Flight f = { 0 };
	Flight* pFlight = &f;


	switch (pComp->sortOpt)
	{
	case eSourceName:
		f.nameSource = getStrExactName("Source airport name?");
		compare = compareFlightBySourceName;
		break;

	case eDestName:
		f.nameDest = getStrExactName("Destination airport name?");
		compare = compareFlightByDestName;
		break;
	
	case eDate:
		getCorrectDate(&f.date);
		compare = compareFlightByDate;
		break;

	case ePlaneCode:
		getPlaneCode(f.thePlane.code);
		compare = compareFlightByPlaneCode;
		break;
	}

	if (compare != NULL)
	{
		Flight** pF = bsearch(&pFlight, pComp->flightArr, pComp->flightCount, sizeof(Flight*), compare);
		if (pF == NULL)
			printf("Flight was not found\n");
		else {
			printf("Flight found, ");
			printFlight(*pF);
		}
	}
	else {
		printf("The search cannot be performed, array not sorted\n");
	}

	if (f.nameSource != NULL)
		free(f.nameSource);

	if (f.nameDest != NULL)
		free(f.nameDest);
}

int	initDateList(Airline* pComp)
{
	if (pComp->flightCount == 0) //no flights!!!
		return 1;

	for (int i = 1; i < pComp->flightCount; i++)
	{
		if (!insertFlightDateToList(&pComp->flighDateList, pComp->flightArr[i]))
			return 0;
	}
	return 1;
}

int insertFlightDateToList(LIST* lst, Flight* pFlight)
{
	int compareRes = 0;

	Date* pDate = &pFlight->date;
	NODE* pNode = &(lst->head);
	
	while (pNode->next != NULL)
	{
		compareRes = compareDate(pDate, pNode->next->key);
		if (compareRes <= 0) // found the place to inset!!
			break;

		pNode = pNode->next;
	}

	if (compareRes < 0  || pNode->next == NULL) //equal will not be inserted!!!
		pNode = insertDateToList(pNode, pDate);

	if (!pNode)
		return 0;
	return 1;
}

NODE* insertDateToList(NODE* pNode, Date* pDate)
{
	Date* pNewDate = (Date*)malloc(sizeof(Date));
	if (!pNewDate)
		return NULL;
	*pNewDate = *pDate;
	
	return  L_insert(pNode, pNewDate);
}

void	freeFlightArr(Flight** arr, int size)
{
	for (int i = 0; i < size; i++)
	{
		freeFlight(arr[i]);
	}
}

eSortOption showSortMenu()
{
	int opt;
	printf("Base on what field do you want to sort?\n");
	do {
		for (int i = 1; i < eNofSortOpt; i++)
			printf("Enter %d for %s\n", i, sortOptStr[i]);
		scanf("%d", &opt);
	} while (opt < 0 || opt >eNofSortOpt);

	return (eSortOption)opt;
}

void	freeCompany(Airline* pComp)
{
	freeFlightArr(pComp->flightArr, pComp->flightCount);
	free(pComp->flightArr);
	free(pComp->name);
	//free the Date* in the list
	L_free(&pComp->flighDateList, freeDate);
}


BOOL readAirlineToCompressBinaryFile(Airline* pComp, FILE* fp)
{
	FILE* f;
	f = fopen(fp, "rb");
	if (!f)
	{
		printf("Error open company file\n");
		return 0;
	}

	BYTE temp[2];
	if (fread(temp, sizeof(BYTE), 2, f) != 2)
	{
		fclose(f);
		return 0;
	}

	int countFlight = (temp[0]) << 1 | ((temp[1] & 0x80) >> 7);
	///*if (countFlight == 1)
	//	countFlight = (int)temp[0] * 2 + 1;
	//else countFlight = temp[0] * 2;*/

	int size = temp[1] & 0xF;

	pComp->name = (char*)calloc((size + 1), sizeof(char));
	if (!pComp->name)
	{
		printf("Error calloc for name airline\n");
		fclose(f);
		return 0;
	}

	if (fread(pComp->name, sizeof(BYTE), size, f) != size)
	{
		printf("Error reading airline name\n");
		free(pComp->name);
		fclose(f);
		return 0;
	}

	pComp->flightCount = countFlight;
	pComp->sortOpt = (temp[1] >> 4) & 0x7;



	pComp->flightArr = (Flight*)malloc(pComp->flightCount * sizeof(Flight));
	if (!pComp->flightArr)
	{
		printf("Error malloc for flight\n");
		free(pComp->name);
		fclose(f);
		return 0;
	}
	for (int i = 0; i < pComp->flightCount; i++)
	{
		pComp->flightArr[i] = (Flight*)malloc(sizeof(Flight));
		if (!readSingleFlightFromCompressFile(pComp->flightArr[i], f))
		{
			printf("Error reading  flight\n");
			free(pComp->name);
			for (int j = 0; j < i; j++)
				freeFlight(pComp->flightArr[j]);
			free(pComp->flightArr);
			fclose(f);
			return 0;
		}
	}
	fclose(f);
	return 1;
}


BOOL readSingleFlightFromCompressFile(Flight* pFilght, FILE* fp)
{
	BYTE data[2];
	if (fread(data, sizeof(BYTE), 2, fp) != 2)
	{
		fclose(fp);
		return 0;
	}

	int sizeSource = (data[0] >> 3) & 0x1F;
	int sizeDest = ((data[0] << 2) & 0x1C) | ((data[1] >> 6) & 0x3);
	int typePlane = (data[1] >> 4) & 0x3;
	int month = (data[1]) & 0xF;

	BYTE data2[3];
	if (fread(data2, sizeof(BYTE), 3, fp) != 3)
	{
		fclose(fp);
		return 0;
	}

	unsigned char codeA = (int)((data2[0] >> 3) & 0x1F) + MIN_CHAR;
	unsigned char codeB = (int)((data2[0] << 2) & 0x1C) | ((data2[1] >> 6) & 0x3) + MIN_CHAR;
	unsigned char codeC = (int)((data2[1] >> 1) & 0x1F) + MIN_CHAR;
	unsigned char codeD = (int)((data2[1] & 0x1) | ((data2[2] >> 4) & 0xF)) + MIN_CHAR;
	int year = ((data[2]) & 0xF) + MIN_YEAR;


	BYTE data3[1];
	if (fread(data3, sizeof(BYTE), 1, fp) != 1)
	{
		fclose(fp);
		return 0;
	}
	int day = (data3[0]) & 0x1F;

	pFilght->nameSource = (char*)calloc(sizeSource + 1, sizeof(char));
	if (!pFilght->nameSource)
		return 0;

	if (fread(pFilght->nameSource, sizeof(BYTE), sizeSource, fp) != sizeSource)
	{
		free(pFilght->nameSource);
		return 0;
	}

	pFilght->nameDest = (char*)calloc(sizeDest + 1, sizeof(char));
	if (!pFilght->nameDest)
		return 0;

	if (fread(pFilght->nameDest, sizeof(BYTE), sizeDest, fp) != sizeDest)
	{
		free(pFilght->nameDest);
		return 0;
	}

	Date d1 = { day, month, year };
	pFilght->date = d1;


	char* fullCode = (char*)calloc(CODE_LENGTH + 1, sizeof(char));
	if (!fullCode)
	{
		return 0;
	}
	fullCode[0] = codeA;
	fullCode[1] = codeB;
	fullCode[2] = codeC;
	fullCode[3] = codeD;


	pFilght->thePlane.type = typePlane;
	strcpy(pFilght->thePlane.code, fullCode);
	free(fullCode);




	return 1;


}

BOOL writeAirlineToCompressBinaryFile(const Airline* pComp)
{
	FILE* f = fopen("airline_compress.bin", "wb");
	if (f == NULL)
		return False;

	BYTE temp[2] = { 0 };
	int i;
	if ((pComp->flightCount % 2) == 0)
		i = 0;
	else (i = 1);

	temp[0] = (pComp->flightCount >> 1);
	temp[1] = (i << 7);
	temp[1] |= (pComp->sortOpt << 4);
	int len = (int)strlen(pComp->name);
	temp[1] |= len;

	if (!fwrite(temp, sizeof(BYTE), 2, f))
	{
		printf("Error write airline to compress file\n");
		fclose(f);
		return False;
	}

	if (fwrite(pComp->name, sizeof(BYTE), len, f) != strlen(pComp->name))
	{
		printf("Error write airline name to copmpress file\n");
		fclose(f);
		return False;
	}

	for (int i = 0; i < pComp->flightCount; i++)
	{
		if (!writeSingleFlightToCompressFile(f, pComp->flightArr[i]))
		{
			printf("Error write flight to copmress file\n");
			fclose(f);
			return False;
		}
	}

	fclose(f);
	printf("Compress Binary file saving was successful\n");
	return 1;
}


BOOL writeSingleFlightToCompressFile(FILE* f, const Flight* pFilght)
{
	BYTE data[2] = { 0 };

	int sizeSource = (int)strlen(pFilght->nameSource);
	int sizeDest = (int)strlen(pFilght->nameDest);

	data[0] |= (sizeSource << 3);
	data[0] |= (sizeDest >> 2);
	data[1] |= (sizeDest << 6);
	data[1] |= (pFilght->thePlane.type << 4);
	data[1] |= pFilght->date.month;

	if (!fwrite(data, sizeof(BYTE), 2, f))
	{
		printf("Error write airline to compress file\n");
		fclose(f);
		return False;
	}

	BYTE data2[3] = { 0 };

	char* fullCode = (char*)calloc(CODE_LENGTH + 1, sizeof(char));
	if (!fullCode)
	{
		return 0;
	}
	strcpy(fullCode, pFilght->thePlane.code);
	int codeA = (unsigned char)fullCode[0] - MIN_CHAR;
	int codeB = (unsigned char)fullCode[1] - MIN_CHAR;
	int codeC = (unsigned char)fullCode[2] - MIN_CHAR;
	int codeD = (unsigned char)fullCode[3] - MIN_CHAR;

	data2[0] |= (codeA << 3);
	data2[0] |= (codeB >> 2);
	data2[1] |= (codeB << 6);
	data2[1] |= (codeC << 1);
	data2[1] |= (codeD >> 4);
	data2[2] |= (codeD << 4);
	int year = pFilght->date.year - MIN_YEAR;
	data2[2] |= year;

	if (!fwrite(data2, sizeof(BYTE), 3, f))
	{
		printf("Error write airline to compress file\n");
		fclose(f);
		return False;
	}
	free(fullCode);

	BYTE data3[1] = { 0 };

	data3[0] |= (pFilght->date.day);

	if (!fwrite(data3, sizeof(BYTE), 1, f))
	{
		printf("Error write airline to compress file\n");
		fclose(f);
		return False;
	}
	if (!fwrite(pFilght->nameSource, sizeof(BYTE), sizeSource, f))
	{
		printf("Error write airline to compress file\n");
		fclose(f);
		return False;
	}

	if (!fwrite(pFilght->nameDest, sizeof(BYTE), sizeDest, f))
	{
		printf("Error write airline to compress file\n");
		fclose(f);
		return False;
	}
	return True;
}

