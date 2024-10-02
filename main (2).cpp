/*
Project: Fitness Tracker
Name: Saad Syed

This is a workout tracker application from which a user can save, modify and delete workout
and exercises, and can also keep track of nutrition 

*/

#include <iostream>
#include <occi.h>
#include <cctype> 
#include <iomanip>
#include <ctime>
#include <string>
#include <sstream>
#include <cstdlib> 
#include <cmath>


using namespace oracle::occi;
using namespace std;

//function signatures
void createUser(Connection *conn);
void homepage(Connection *conn, string user, string userID, string diettype,int height,int weight);
void login(Connection *conn);
void workouts(Connection *conn, string username, string userID);
void Exercise(Connection *conn, string workoutName,string workoutID);
void nutritionTracking(Connection *conn, string userid, int weight,int height, string diettype);
string generateRandomNumber(Connection *conn);
string calculateEndDate(int goalDays);
string readPassword();
string generateRandomExerciseID(Connection *conn);
string generateNutritionID(Connection *conn);




int main(){
    //random number fo id
    srand(time(0));
    int randomNumber = rand() % 100 + 1;

    
    //creating environment
    Environment *env = Environment::createEnvironment(Environment::DEFAULT);
    string username, password;
    cout<<"To access DB, enter "<<endl;
    cout<<"Enter you CSCI username"<<endl;
    cin>>username;
    cout<<"Enter password"<<endl;
    cin>>password;
    Connection *conn = env->createConnection(username, password, "database.csci.viu.ca");


    //login UI
    bool flag;
    flag=true;
    while(flag==true){
        system("clear");
        cout<<"WELCOME TO FITNESS TRACKER"<<endl;
        string choice;
        cout<<" if you are new and dont have an account, press 1"<<endl;
        cout<<" if you alrady have account, press 2"<<endl;
        cin>>choice;
        
        if (choice == "1"){
            flag=false;
            //creates an user
            createUser(conn);
        }
        else if (choice == "2"){
            flag=false;

            //takes to login function
            login(conn);
        }
        else if (choice =="3"){
            cout<<"PROGRAM TERMINATING"<<endl;
            return 0;

        }    
        else{
            cout<<"invalid choice try again"<<endl;
            system("clear");
        }
    }
    
    env->terminateConnection(conn);
    Environment::terminateEnvironment(env);
    return 0;
}

/*
function will create a user in the UUSER db
use prepared statements and insert data
@param conn: it is the pointer pointing to the connection of a the environmnet
*/
void createUser(Connection *conn) {
    cout << "To create a user, please enter the following information" << endl;
    cout << "------------------------------------" << endl;

    string uid;
    bool uidExists = false;
    Statement *stmt = conn->createStatement("SELECT COUNT(*) FROM UUser WHERE UserID = :1");
    string insertQuery = "INSERT INTO UUser (UserID, Username, Email, Password, Age, Gender, Height, Weight, diettype) VALUES (:1, :2, :3, :4, :5, :6, :7, :8, :9)";
    Statement *pstmt = conn->createStatement(insertQuery);


    
    do {
        cout << "Enter a unique UID: ";
        cin >> uid;

        // Check if the UID already exists in the database
        try {
            stmt->setString(1, uid);
            ResultSet *rs = stmt->executeQuery();
            if (rs->next()) {
                int count = rs->getInt(1);
                if (count > 0) {
                    cout << "UID already exists. Please enter a different UID." << endl;
                    uidExists = true;
                } else {
                    uidExists = false;
                }
            }
            conn->terminateStatement(stmt);
        } catch (SQLException &ex) {
            cout << "Error executing SQL: " << ex.what() << endl;
            // do i need rollback here?
        }
    } while (uidExists);

    string u, email, p, age, gender, mcb;
    int height, weight;

    cout << "Enter username: ";
    cin >> u;
    cout << "Enter email: ";
    cin >> email;
    cout << "Enter password: ";
    cin >> p;
    cout << "Enter age: ";
    cin >> age;
    cout << "Enter gender: ";
    cin >> gender;
    cout << "Enter height in cm: ";
    cin >> height;
    cout << "Enter weight in kg: ";
    cin >> weight;
    cout << "Enter diet type (cut, bulk, or maintainance): ";
    cin >> mcb;

    // Use prepared statement to insert user data into the database
    pstmt->setString(1, uid);
    pstmt->setString(2, u);
    pstmt->setString(3, email);
    pstmt->setString(4, p);
    pstmt->setString(5, age);
    pstmt->setString(6, gender);
    pstmt->setInt(7, height);
    pstmt->setInt(8, weight);
    pstmt->setString(9, mcb);

    // Execute the update statement
    int Rows = pstmt->executeUpdate();
    // Terminate the prepared statement
    

    // Check if the update was successful
    if (Rows > 0) {
        cout << "Insertion Successful" << endl; 
        conn->commit();
        conn->terminateStatement(pstmt);
    } else {
        cout << "Insertion Failed" << endl;
        conn->rollback();
    }

    // Redirect user to homepage after creating an account 
    system("clear");
    homepage(conn, u, uid, mcb, height, weight);
}

/*
homepage function allows user to track nutrition, view workouts and logout
@param conn: It is the connectiontion to the environment
@param user: name of the user
@param userID: UID of the user 
@param diettype: diet goal of the user(cut/maintainance/bulk)
@param height: height of the user in cm
@weight: weight of the user in kg
*/
void homepage(Connection *conn, string user, string uid, string mcb, int height, int weight) {
    bool loggedIn = true;

    while (loggedIn) {
        // Clear the screen
        system("clear");

        // Display menu options
        cout << "HELLO! " << user << " which one would you like to choose?" << endl;
        cout << " press 1 to track nutrition" << endl;
        cout << " press 2 to view workouts" << endl;
        cout << " press 3 to logout" << endl;
        cout << "Enter your choice: ";
        int choice;
        cin >> choice;

        // Process user choice
        switch (choice) {
            case 1:
                loggedIn = false;
                // Function to track nutrition
                cout << "Tracking nutrition..." << endl;
                system("clear");
                nutritionTracking(conn, uid, weight, height, mcb);
                break;
            case 2:
                loggedIn = false;
                // Function to view workouts
                cout << "Viewing workouts..." << endl;
                system("clear");
                workouts(conn,user,uid);
                break;
            case 3:
                // Logout
                loggedIn = false;
                cout << "Logging out..." << endl;
                break;
            default:
                // Invalid choice
                cout << "Invalid choice! Please try again." << endl;
                system("clear");

                break;
        }

    }

}

/*
Function checks the existance of user and logs in into the application
@param conn: connection to the environment
*/
void login(Connection *conn) { 

    string username, password;
    cout << "Enter your username: ";
    cin >> username;
    cout << "Enter your password: ";
    cin >> password;

    string Query;
    Query = Query+"SELECT userID,diettype,weight,height FROM UUser u WHERE u.username = :1 AND u.password = :2";
    
    Statement *stmt = conn->createStatement(Query);
    stmt->setString(1, username);
    stmt->setString(2, password);
    ResultSet *rs = stmt->executeQuery(); 
    int count = 0;
    string userID, diettype;
    int weight,height;
    while (rs->next()) { 
        count++;
        userID = rs->getString(1);
        diettype = rs->getString(2);
        weight = rs->getInt(3);
        height = rs->getInt(4);

    }

    // Clean up resources
    stmt->closeResultSet(rs);
    conn->terminateStatement(stmt);

    //if user exists
    if(count >= 1) {
        homepage(conn, username, userID,diettype,height, weight);
    } 
    
    //if not
    else {
        cout << "Incorrect information, credentials wrong or the user doesn't exist" << endl;
    }
}
/*
workout function allows user to interact with workouts
@param conn: It is the connectiontion to the environment
@param user: name of the user
@param userID: UID of the user 
*/
void workouts(Connection *conn, string username, string userID) {
    bool flag = true;
    string Query = "SELECT workoutname FROM workouts WHERE userid = :1";
    Statement *listStmt = conn->createStatement(Query);

    Query = "INSERT INTO Workouts (WorkoutID, UserID, WorkoutName, Description, Goal, EndDate) "
             "VALUES (:1, :2, :3, :4, :5, TO_DATE(:6, 'YYYY-MM-DD'))";
    Statement *insertStmt = conn->createStatement(Query);

    Query = "SELECT WorkoutID FROM Workouts WHERE WorkoutName = :1 AND UserID = :2";
    Statement *retrieveStmt = conn->createStatement(Query);

    Query = "SELECT COUNT(*) FROM workouts WHERE userid = :1 AND workoutname = :2";
    Statement *checkStmt = conn->createStatement(Query);

    Query = "DELETE FROM workouts WHERE userid = :1 AND workoutname = :2";
    Statement *deleteStmt = conn->createStatement(Query);

    Query = "DELETE FROM WorkoutExercises WHERE WorkoutID = :1";
    Statement *deleteExercisesStmt = conn->createStatement(Query);

    Query = "UPDATE workouts SET workoutname = :1 WHERE userid = :2 AND workoutname = :3";
    Statement *updateStmt = conn->createStatement(Query);

    while (flag) {
        system("clear");
        int choice;

        // Display the list of workouts of the user
        listStmt->setString(1, userID);
        ResultSet *listRs = listStmt->executeQuery();

        cout << "List of Workouts:" << endl;
        // Add serial number next to name
        int serialNumber = 1;
        while (listRs->next()) {
            cout << serialNumber << ". " << listRs->getString(1) << endl;
            serialNumber++;
        }

        // Display options for workout management
        cout << "\nWorkout Options:" << endl;
        cout << "1. Add New Workout" << endl;
        cout << "2. View Workout" << endl;
        cout << "3. Delete Workout" << endl;
        cout << "4. Rename Workout" << endl;
        cout << "5. Quit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        switch (choice) {

            // Add New Workout
            case 1:{
                string workoutName, description;
                int goal;
                cout << "Enter the name for the new workout: ";
                cin.ignore(); // Ignore newline character from previous input
                getline(cin, workoutName);
                cout << "Enter a description for the workout: ";
                getline(cin, description);
                cout << "Enter the goal for the workout (in days): ";
                cin >> goal;
                string endDate = calculateEndDate(goal); // Calculate the end date

                // Insert the new workout into the database
                
                insertStmt->setString(1, generateRandomNumber(conn)); 
                insertStmt->setString(2, userID);
                insertStmt->setString(3, workoutName);
                insertStmt->setString(4, description);
                insertStmt->setInt(5, goal);
                insertStmt->setString(6, endDate);

                int rowsInserted = insertStmt->executeUpdate();

            if (rowsInserted > 0) {
                cout << "New workout '" << workoutName << "' added successfully." << endl;
                conn->commit(); 
            } else {
                cout << "Failed to add the new workout." << endl;
                conn->rollback();
            }

 
            break;
        
        }
        // View Workout
        case 2: {
            string workoutName;
            cout << "Enter the name of the workout to view: ";
            cin.ignore(); // Ignore newline character from previous input
            getline(cin, workoutName);

            // Query to retrieve the workoutID based on workoutName and userID
            retrieveStmt->setString(1, workoutName);
            retrieveStmt->setString(2, userID);
            ResultSet *rs = retrieveStmt->executeQuery();

            string workoutID; // Variable to store the retrieved workoutID

            if (rs->next()) {
                workoutID = rs->getString(1); // Store the retrieved workoutID
                cout << "Editing workout '" << workoutName << "'." << endl;
                // You can call the workoutExercises function here and pass workoutName and workoutID
                Exercise(conn, workoutName, workoutID);
            } else {
                cout << "Workout '" << workoutName << "' not found." << endl;
            }

            // Clean up resources
            retrieveStmt->closeResultSet(rs);
            break;
        
        }
        case 3: {
                string workoutNameToDelete;
                cout << "Enter the name of the workout to delete: ";
                cin.ignore(); // Ignore newline character from previous input
                getline(cin, workoutNameToDelete);

                // Check if the workout exists
                checkStmt->setString(1, userID);
                checkStmt->setString(2, workoutNameToDelete);
                ResultSet *checkRs = checkStmt->executeQuery();

                int workoutCount = 0;
                if (checkRs->next()) {
                    workoutCount = checkRs->getInt(1);
                }

                if (workoutCount > 0) {
                    // Confirm deletion with user
                    char confirmDelete;
                    cout << "Are you sure you want to delete the workout '" << workoutNameToDelete << "'? (Y/N): ";
                    cin >> confirmDelete;

                    if (toupper(confirmDelete) == 'Y') {
                        // Delete associated exercises first
                        retrieveStmt->setString(1, workoutNameToDelete);
                        retrieveStmt->setString(2, userID);
                        ResultSet *rs = retrieveStmt->executeQuery();

                        string workoutID; // Variable to store the retrieved workoutID

                        if (rs->next()) {
                            workoutID = rs->getString(1); // Store the retrieved workoutID
                            deleteExercisesStmt->setString(1, workoutID);
                            deleteExercisesStmt->executeUpdate();
                        }

                        // Then delete the workout from the database
                        deleteStmt->setString(1, userID);
                        deleteStmt->setString(2, workoutNameToDelete);

                        int rowsDeleted = deleteStmt->executeUpdate();

                        if (rowsDeleted > 0) {
                            cout << "Workout '" << workoutNameToDelete << "' deleted successfully." << endl;
                            conn->commit(); // Commit the transaction
                        } else {
                            cout << "Failed to delete the workout." << endl;
                            conn->rollback();
                        }
                    }
                } else {
                    cout << "Workout '" << workoutNameToDelete << "' not found." << endl;
                }

                // Clean up resources
                checkStmt->closeResultSet(checkRs);
                break;
            }
        // Rename Workout
        case 4:{
            string oldWorkoutName, newWorkoutName;
            cout << "Enter the name of the workout to rename: ";
            cin.ignore(); // Ignore newline character from previous input
            getline(cin, oldWorkoutName);
            cout << "Enter the new name for the workout: ";
            getline(cin, newWorkoutName);

            // Check if the workout exists
            checkStmt->setString(1, userID);
            checkStmt->setString(2, oldWorkoutName);
            ResultSet *checkRs = checkStmt->executeQuery();

            int workoutCount = 0;
            if (checkRs->next()) {
                workoutCount = checkRs->getInt(1);
            }

            if (workoutCount > 0) {
                // Update the workout name
                updateStmt->setString(1, newWorkoutName);
                updateStmt->setString(2, userID);
                updateStmt->setString(3, oldWorkoutName);

                int rowsUpdated = updateStmt->executeUpdate();
               

                if (rowsUpdated > 0) {
                    cout << "Workout '" << oldWorkoutName << "' renamed to '" << newWorkoutName << "'." << endl;
                    conn->commit(); // Commit the transaction
                } else {
                    cout << "Failed to rename the workout." << endl;
                    conn->rollback();
                }
                
            } 
            else {
                cout << "Workout '" << oldWorkoutName << "' not found." << endl;
            }

            // Clean up resources
            checkStmt->closeResultSet(checkRs);
            break;
        }

        case 5: // Quit
            flag = false;
            cout << "Exiting workout management." << endl;
            break;

        default:
            cout << "Invalid choice" << endl;
            break;
    }
    }
    conn->terminateStatement(listStmt);
    conn->terminateStatement(insertStmt);
    conn->terminateStatement(retrieveStmt);
    conn->terminateStatement(checkStmt);
    conn->terminateStatement(deleteStmt);
    conn->terminateStatement(deleteExercisesStmt);
    conn->terminateStatement(updateStmt);
}

/*
Exercise function shows all the exercises of a workout of a particular user and enables
user to modify or view them
@param conn: connection to the environment
@param workoutName: name of the workout whcih contains the exercises
@param workoutID: workoiut ID of the workout 
*/
void Exercise(Connection *conn, string workoutName,string workoutID){
    bool flag =true;
        // Define and assign queries before the loop
    string Query;
    Query = "SELECT ExerciseName, Sets, Reps, Weight, highscore FROM WorkoutExercises WHERE WorkoutID = :1";
    Statement *selectStmt = conn->createStatement(Query);

    Query = "INSERT INTO WorkoutExercises (WorkoutExerciseID, WorkoutID, ExerciseName, Sets, Reps, Weight, HighScore) "
                         "VALUES (:1, :2, :3, :4, :5, :6, :7)";
    Statement *insertStmt = conn->createStatement(Query);

    Query = "DELETE FROM WorkoutExercises WHERE ExerciseName = :exerciseName";
    Statement *deleteStmt = conn->createStatement(Query);

    // Create statements outside the loop
    while(flag==true){
    system("clear");

        // Display exercise information
        cout << "Exercise Information for Workout '" << workoutName << "':" << endl;
        selectStmt->setString(1, workoutID);
        ResultSet *rs = selectStmt->executeQuery();

        cout << "Exercise Name\tSets\tReps\tWeight\tHigh Score" << endl;
        cout << "------------------------------------" << endl;
        while (rs->next()) {
            cout << rs->getString(1) << "\t" << rs->getString(2) << "\t" << rs->getString(3) << "\t" << rs->getString(4)<< "\t" << rs->getString(5) << endl;
        }

        selectStmt->closeResultSet(rs);

        // Display exercise management options
        int choice;
        cout << "\nExercise Management Options:" << endl;
        cout << "1. Add Exercise" << endl;
        cout << "2. Edit Exercise" << endl;
        cout << "3. Delete Exercise" << endl;
        cout << "4. Go Back" << endl;
        cout << "Enter your choice: ";
        cin >> choice;


  switch(choice) {

    //Add
    case 1:{
        string name;
        int sets, weight, rep; 
        cout << "Enter the exercise name: ";
        getline(cin >> ws, name); // Read exercise name with spaces

        cout << "Enter the number of sets: ";
        cin >> sets;

        cout << "Enter the weight (in kg): ";
        cin >> weight;

        cout << "Enter the number of reps: ";
        cin >> rep;

        insertStmt->setString(1, generateRandomExerciseID(conn)); 
        insertStmt->setString(2, workoutID); 
        insertStmt->setString(3, name);
        insertStmt->setInt(4, sets);
        insertStmt->setInt(5, rep); 
        insertStmt->setInt(6, weight);
        insertStmt->setInt(7, (weight+1)); //highscore will be the same as current weight and user can edit it later

        int Rows = insertStmt->executeUpdate();

        // Check if the update was successful
        if (Rows > 0) {
            cout << "Insertion Successful" << endl; 
            conn->commit();
        } else {
            cout << "Insertion Failed" << endl;
            conn->rollback();
        }
        break;
    }

    //Edit 
    case 2: {
    string exerciseName, columnName, value;
    cout << "Enter the exercise name you want to edit: ";
    cin.ignore(); // Ignore newline character from previous input
    getline(cin, exerciseName);

    cout << "Enter the exercise column you want to edit: ";
    getline(cin, columnName);

    cout << "Enter the value: ";
    getline(cin, value);

    //Query has been implemented inside the test case to access column name
    //this query will follow the principle of "create query once" as this case will be active only when user decides to use it
    string updateQuery = "UPDATE WorkoutExercises SET " + columnName + " = :value WHERE ExerciseName = :exerciseName AND WorkoutID = :workoutID";
    Statement *updateStmt = conn->createStatement(updateQuery);

    // Bind the parameters for the update query
    updateStmt->setString(1, value);
    updateStmt->setString(2, exerciseName);
    updateStmt->setString(3, workoutID);

    // Execute the update query
    int Rows = updateStmt->executeUpdate();

    // Check if the update was successful
    if (Rows > 0) {
        cout << "Update Successful" << endl;
        conn->commit();
    } else {
        cout << "Update Failed: Exercise not found or column name incorrect" << endl;
        conn->rollback();
    }

    conn->terminateStatement(updateStmt);
    break;
}

    //delete
    case 3: {
        string exerciseNameToDelete;
        cout << "Enter the exercise name you want to delete: ";
        cin.ignore(); // Ignore newline character from previous input
        getline(cin, exerciseNameToDelete);

        // Bind the exercise name to the parameter
        deleteStmt->setString(1, exerciseNameToDelete);

        // Execute the delete query
        int Rows = deleteStmt->executeUpdate();

        // Check if the delete was successful
        if (Rows > 0) {
            cout << "Delete Successful" << endl;
            conn->commit();
        } else {
            cout << "Delete Failed: Exercise not found" << endl;
            conn->rollback();
        }

         break;
    }
   case 4: // Go Back
                flag = false;
                break;

    default:
        cout << "Invalid choice" << endl;
    }
    }
    conn->terminateStatement(selectStmt);
    conn->terminateStatement(insertStmt);
    conn->terminateStatement(deleteStmt);
}
/*
Function used to generate a unique random number for Nutrition Tracking Function
@param conn: connection to the database
*/
string generateNutritionID(Connection *conn) {
    string numString, Query;
    Statement *stmt = NULL;
    ResultSet *rs = NULL;
    Query = "SELECT COUNT(*) FROM NutritionTracking WHERE trackingID = :1";
    stmt = conn->createStatement(Query);

    do {
        // Seed the random number generator with the current time
        srand(time(0));

        // Generate a random number between 1 and 100
        int num = rand() % 100 + 1;
        stringstream ss;
        ss << num;
        numString = ss.str(); 


        try {
            // Check if the generated number exists in the Workouts table
            stmt->setString(1, numString);
            rs = stmt->executeQuery();
            if (rs->next()) {
                int count = rs->getInt(1);
                if (count == 0) {
                    // The generated number does not exist in the database, so it's unique
                    break;
                }
            }
        } catch (SQLException &ex) {
            cout << "Error executing SQL: " << ex.what() << endl;
        }
        // Cleanup resources
        if (rs) {
            conn->terminateStatement(stmt);
            stmt = NULL;
        }
    } while (true);

    // Cleanup resources
    if (rs) {
        conn->terminateStatement(stmt);
    }

    return numString;
}

/*
Function used to generate a unique random number which will be used as primary key
@param conn: connection to the database
*/
string generateRandomNumber(Connection *conn) {
    string numString, Query;
    Statement *stmt = NULL;
    ResultSet *rs = NULL;
    Query = "SELECT COUNT(*) FROM Workouts WHERE WorkoutID = :1";
    stmt = conn->createStatement(Query);

    do {
        // Seed the random number generator with the current time
        srand(time(0));

        // Generate a random number between 1 and 100
        int num = rand() % 100 + 1;
        stringstream ss;
        ss << num;
        numString = ss.str(); 


        try {
            // Check if the generated number exists in the Workouts table
            stmt->setString(1, numString);
            rs = stmt->executeQuery();
            if (rs->next()) {
                int count = rs->getInt(1);
                if (count == 0) {
                    // The generated number does not exist in the database, so it's unique
                    break;
                }
            }
        } catch (SQLException &ex) {
            cout << "Error executing SQL: " << ex.what() << endl;
        }
        // Cleanup resources
        if (rs) {
            conn->terminateStatement(stmt);
            stmt = NULL;
        }
    } while (true);

    // Cleanup resources
    if (rs) {
        conn->terminateStatement(stmt);
    }

    return numString;
}
/*
function calculates end date from the time the atribute is created + goal in days
@param goalDays: will be used to add into the processed value
*/
string calculateEndDate(int goalDays) {
    time_t now = time(0); // Get the current time
    struct tm currentTime = *localtime(&now); // Convert to local time struct

    // Add the goal days to the current date
    currentTime.tm_mday += goalDays;
    mktime(&currentTime); // Normalize the time struct

    // Format the end date as YYYY-MM-DD for SQL
    char endDateStr[11];
    strftime(endDateStr, sizeof(endDateStr), "%Y-%m-%d", &currentTime);

    return endDateStr;
}

/*
Function tracks and displays the nutritional information of the user
@param conn: connection to the environment
@param username: username of the user
@param weight: weight of the user in kg
@param height: height of the user in cm
@param dietType: dietType of the user
*/

void nutritionTracking(Connection *conn, string userid, int weight, int height, string diettype) {
    int calories;
    cout << "Enter the amount of calories you consumed today: ";
    cin >> calories;

    // Calculate BMI
    double heightMeters = height / 100.0; // Convert height to meters
    double bmi = weight / pow(heightMeters, 2);

    string weightStatus;
    if (bmi < 18.5) {
        weightStatus = "Underweight";
    } else if (bmi >= 18.5 && bmi < 24.9) {
        weightStatus = "Normal Weight";
    } else if (bmi >= 24.9 && bmi < 29.9) {
        weightStatus = "Overweight";
    } else {
        weightStatus = "Obese";
    }

    // Print weight status and BMI
    cout << "BMI: " << bmi << " (" << weightStatus << ")" << endl;

    // Calculate target calories based on diet type
    int targetCalories;
    if (diettype == "cut") {
        targetCalories = weight * 2.2 * 12; // Adjusted for cutting phase
    } else if (diettype == "bulk") {
        targetCalories = weight * 2.2 * 16; // Adjusted for bulking phase
    } else if (diettype == "maintenance") {
        targetCalories = weight * 2.2 * 14; // Adjusted for maintenance phase
    } else {
        cout << "Invalid diet type." <<endl;
        return;
    }

    // Calculate remaining calories to be consumed
    int remainingCalories = targetCalories - calories;

    double proteinRatio = 0.3; // 30% of calories from protein
    double fatRatio = 0.3; // 30% of calories from fat
    double carbRatio = 0.4; // 40% of calories from carbs

    // Calculate grams of each macronutrient
    double proteinCalories = targetCalories * proteinRatio;
    double fatCalories = targetCalories * fatRatio;
    double carbCalories = targetCalories * carbRatio;

    double proteinGrams = proteinCalories / 4.0; // 1 gram of protein = 4 calories
    double fatGrams = fatCalories / 9.0; // 1 gram of fat = 9 calories
    double carbGrams = carbCalories / 4.0; // 1 gram of carb = 4 calories

    cout << "Target Calories: " << targetCalories << endl;
    cout << "Remaining Calories to Consume: " << remainingCalories << endl;
    if (remainingCalories > 0) {
        cout << "You need to consume " << remainingCalories << " more calories." << endl;
    } else if (remainingCalories == 0) {
        cout << "You ate the perfect amount of food for the day." << endl;
    } else {
        cout << "You overate." << endl;
    }
    cout<<"Macros recommendation based on total calories to be consumed"<<endl;
    cout << "Protein: " << proteinGrams << " grams" << endl;
    cout << "Fat: " << fatGrams << " grams" << endl;
    cout << "Carbohydrates: " << carbGrams << " grams" << endl;

    // Update the NutritionTracking table with the calories consumed and target calories
    string query = "INSERT INTO NutritionTracking (trackingID, userID, CaloriesConsumed, CaloriesTarget) "
                        "VALUES (:1, :2, :3, :4)";
    Statement *stmt = conn->createStatement(query);
    stmt->setString(1, generateNutritionID(conn)); // Assuming generateNutritionID() generates a unique TrackingID
    stmt->setString(2, userid);
    stmt->setInt(3, calories);
    stmt->setInt(4, targetCalories);
    int rowsInserted = stmt->executeUpdate();

    if (rowsInserted > 0) {
        cout << "Nutritional tracking information updated successfully." << endl;
        conn->commit();
    } else {
        cout<< "Error updating nutritional tracking information." << endl;
        conn->rollback();
    }

    conn->terminateStatement(stmt);
}

/*
function used to generate random number as primary key for exercise function
@param conn: connection to the databse
*/
string generateRandomExerciseID(Connection *conn) {
    string exerciseID;
    Statement *stmt = NULL;
    ResultSet *rs = NULL;
    stmt = conn->createStatement("SELECT COUNT(*) FROM WorkoutExercises WHERE WorkoutExerciseID = :1");
    stmt->setString(1, exerciseID);

    do {
        srand(time(0));

        // Generate a random number between 1 and 100
        int num = rand() % 1000 + 1;
        
        stringstream ss;
        ss << num;
        exerciseID = ss.str(); 
        try {
            // Check if the generated exercise ID exists in the WorkoutExercises table
            rs = stmt->executeQuery();
            if (rs->next()) {
                int count = rs->getInt(1);
                if (count == 0) {
                    // The generated exercise ID does not exist in the database, so it's unique
                    break;
                }
            }
        } catch (SQLException &ex) {
            cout << "Error executing SQL: " << ex.what() << endl;
        }

        // Cleanup resources
        if (rs) {
            conn->terminateStatement(stmt);
            stmt = NULL;
        }
    } while (true);

    // Cleanup resources
    if (rs) {
        conn->terminateStatement(stmt);
    }

    return exerciseID;
}









