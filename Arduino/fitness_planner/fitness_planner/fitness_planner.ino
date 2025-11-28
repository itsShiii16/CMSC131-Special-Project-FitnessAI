#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "model_v1.h"
#include "lookup_table.h"

using Eloquent::ML::Port::DecisionTree;
DecisionTree clf;

// LCD setup (20x4)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ---- Helper: read a number from Serial safely ----
float readNumber(const char* label) {
  Serial.print(label);
  Serial.print(": ");

  // Clear any leftover bytes in buffer
  while (Serial.available()) {
    Serial.read();
  }

  // Wait for NEW input from user
  while (!Serial.available()) {
    // just wait
  }

  // Read a full line, then convert to float
  String input = Serial.readStringUntil('\n');
  input.trim();
  float val = input.toFloat();

  // Echo back to Serial so user sees what was read
  Serial.println(val);
  return val;
}

// ---- Helper: convert plan_key into 3 short LCD lines ----
void interpretPlan(const char* key, const char*& mode, const char*& freq, const char*& dur) {
  mode = "Custom training";
  freq = "3-4 days / week";
  dur  = "30-45 min / day";

  if (strcmp(key, "C_CONS_3D_20_30") == 0) {
    mode = "Cardio (light)";
    freq = "3 days / week";
    dur  = "20-30 min / day";
  }
  else if (strcmp(key, "C_MOD_3_4D_30_40") == 0) {
    mode = "Cardio (moderate)";
    freq = "3-4 days / week";
    dur  = "30-40 min / day";
  }
  else if (strcmp(key, "C_STD_4D_35_45") == 0) {
    mode = "Cardio (steady)";
    freq = "4 days / week";
    dur  = "35-45 min / day";
  }
  else if (strcmp(key, "S_CONS_2_3D_30_40") == 0) {
    mode = "Strength (easy)";
    freq = "2-3 days / week";
    dur  = "30-40 min / day";
  }
  else if (strcmp(key, "S_MOD_3D_35_50") == 0) {
    mode = "Strength (moderate)";
    freq = "3 days / week";
    dur  = "35-50 min / day";
  }
  else if (strcmp(key, "S_STD_3D_45_60") == 0) {
    mode = "Strength (intense)";
    freq = "3-4 days / week";
    dur  = "45-60 min / day";
  }
}

// ---- Helper: BMI category calculation ----
const char* getBMICategory(float bmi) {
  if (bmi < 18.5) {
    return "Underweight";
  } else if (bmi >= 18.5 && bmi < 24.9) {
    return "Normal weight";
  } else if (bmi >= 25 && bmi < 29.9) {
    return "Overweight";
  } else {
    return "Obese";
  }
}

// ---- Helper: standby / greeting screen & wait for "1" ----
void waitForUserToStart() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("Fitness AI Planner");
  lcd.setCursor(0, 2);
  lcd.print("Press 1 in Serial");
  lcd.setCursor(0, 3);
  lcd.print("to start planning");

  Serial.println("=== Fitness AI Planner ===");
  Serial.println("Press 1 and Enter to start a new fitness plan.");
  Serial.println();

  // Clear any leftover bytes
  while (Serial.available()) {
    Serial.read();
  }

  // Wait until user sends '1'
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '1') {
        Serial.println("Starting new plan...");
        break;
      }
    }
  }

  // Optional: small visual pause
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Let's plan your");
  lcd.setCursor(0, 1);
  lcd.print("fitness journey!");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("I need your:");
  lcd.setCursor(0, 1);
  lcd.print("Age, Height, Weight");
  lcd.setCursor(0, 2);
  lcd.print("Gender, Goal");
  lcd.setCursor(0, 3);
  lcd.print("Follow Serial steps");
  delay(3000);
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  // First time power on: go directly to standby screen
  waitForUserToStart();
}

// ---- Helper: read a number from Serial safely with input validation ----
float readValidInput(const char* label, const char* errorMessage, float minValue) {
  float val = 0;
  while (val <= minValue) {
    val = readNumber(label);
    if (val <= minValue) {
      Serial.println(errorMessage);
    }
  }
  return val;
}

void loop() {
  waitForUserToStart();

  // ---- INPUT PHASE (with LCD hints + Serial input) ----

  // 3.1 Age
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Step 1: Age");
  lcd.setCursor(0, 1);
  lcd.print("Enter in Serial...");
  float age = readValidInput("Enter Age (years)", "Age must be greater than 0", 0);

  // 3.2 Height
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Step 2: Height");
  lcd.setCursor(0, 1);
  lcd.print("in meters (eg. 1.70)");
  float height = readValidInput("Enter Height (m)", "Height must be greater than 0", 0);

  // 3.3 Weight
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Step 3: Weight");
  lcd.setCursor(0, 1);
  lcd.print("in kg (e.g. 70)");
  float weight = readValidInput("Enter Weight (kg)", "Weight must be greater than 0", 0);

  // 3.4 Gender
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Step 4: Gender");
  lcd.setCursor(0, 1);
  lcd.print("0=Female  1=Male");
  int gender = (int) readValidInput("Enter Gender (0=F,1=M)", "Enter 0 or 1", -1);

  // 3.5 Goal
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Step 5: Goal");
  lcd.setCursor(0, 1);
  lcd.print("0=Fat loss 1=Muscle");
  int goal = (int) readValidInput("Enter Goal (0=Fat,1=Mus)", "Enter 0 or 1", -1);

  // 4) Motivation message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nice work!");
  lcd.setCursor(0, 1);
  lcd.print("You're taking a");
  lcd.setCursor(0, 2);
  lcd.print("step for your");
  lcd.setCursor(0, 3);
  lcd.print("health. :)");
  delay(2500);

  // 5) Calculating fitness plan
  float bmi = weight / (height * height);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calculating your");
  lcd.setCursor(0, 1);
  lcd.print("fitness plan...");
  lcd.setCursor(0, 2);
  lcd.print("BMI: ");
  lcd.print(bmi, 1);  // Show BMI value with 1 decimal point

  // Get the BMI category
  const char* bmiCategory = getBMICategory(bmi);

  // Display the BMI category (on row 3 or 4)
  lcd.setCursor(0, 3);  // Position on the 4th row
  lcd.print("Category: ");
  lcd.print(bmiCategory);  // Display BMI category (Underweight, Normal, etc.)

  delay(3000);  // Keep the results on screen for 3 seconds

  // Prepare features: [Age, gender_enc, BMI, goal_enc]
  float x[4] = {
    age,
    (float) gender,
    bmi,
    (float) goal
  };

  // Run TinyML model
  int idx = clf.predict(x);

  const int numClasses = sizeof(PLAN_KEY_TABLE) / sizeof(PLAN_KEY_TABLE[0]);
  const char* planKey = PLAN_KEY_TABLE[idx];

  // Interpret plan into 3 short lines
  const char* mode;
  const char* freq;
  const char* dur;
  interpretPlan(planKey, mode, freq, dur);

  // 6 & 7) Display interpreted plan on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Your results say:");

  lcd.setCursor(0, 1);
  lcd.print(mode);   // e.g., "Strength (moderate)"

  lcd.setCursor(0, 2);
  lcd.print(freq);   // e.g., "3-4 days / week"

  lcd.setCursor(0, 3);
  lcd.print(dur);    // e.g., "30-45 min / day"

  // Also log details to Serial for checking
  Serial.println("\n====== Fitness AI Result ======");
  Serial.print("Age: ");    Serial.println(age);
  Serial.print("Gender: "); Serial.println(gender);
  Serial.print("Height: "); Serial.println(height, 2);
  Serial.print("Weight: "); Serial.println(weight, 1);
  Serial.print("BMI: ");    Serial.println(bmi, 1);
  Serial.print("Plan key: ");
  Serial.println(planKey);
  Serial.print("Mode: ");
  Serial.println(mode);
  Serial.print("Frequency: ");
  Serial.println(freq);
  Serial.print("Duration: ");
  Serial.println(dur);
  Serial.println("================================\n");

  // Hold result on screen for 10 seconds
  delay(15000);

  // Clear LCD; next iteration of loop() goes back to waitForUserToStart()
  lcd.clear();
}
