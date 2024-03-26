#include "socialnetworkwindow.h"
#include "ui_socialnetworkwindow.h"
#include "network.h"
#include "user.h"
#include <QFileDialog>
#include <cstdio>
#include <sstream>

using namespace std;

SocialNetworkWindow::SocialNetworkWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SocialNetworkWindow)
{
    ui->setupUi(this);
    displayLoginPage();

    string userfile = "/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/users.txt";
    char *userfileCStr = userfile.data();

    string postsfile = "/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/posts.txt";
    char *postfileCStr = postsfile.data();

    if(net.readUsers(userfileCStr) == 0 && net.readPosts(postfileCStr) == 0) {
        cout << "Success reading in files";
    } else {
        cout << "Error reading in files";
    }

    connect(ui->loginButton, &QPushButton::clicked, this, &SocialNetworkWindow::loginButtonCallback);
    connect(ui->backButton, &QPushButton::clicked, this, &SocialNetworkWindow::backButtonCallback);
    connect(ui->addFriendButton, &QPushButton::clicked, this, &SocialNetworkWindow::addButtonCallback);
    connect(ui->signupButton, &QPushButton::clicked, this, &SocialNetworkWindow::signupButtonCallback);
    connect(ui->signupOKButton, &QPushButton::clicked, this, &SocialNetworkWindow::signupOKCallback);
    connect(ui->signupbackButton, &QPushButton::clicked, this, &SocialNetworkWindow::signupBackButtonCallback);
    connect(ui->writePostButton, &QPushButton::clicked, this, &SocialNetworkWindow::writePostCallback);
    connect(ui->signupUploadButton, &QPushButton::clicked, this, &SocialNetworkWindow::uploadCallback);
    connect(ui->searchButton, &QPushButton::clicked, this, &SocialNetworkWindow::searchCallback);

    connect(ui->friendsTable, &QTableWidget::cellClicked, this, &SocialNetworkWindow::friendTableCallback);
    connect(ui->friendSuggestionsTable, &QTableWidget::cellClicked, this, &SocialNetworkWindow::suggestionTableCallback);
}

void SocialNetworkWindow::signupButtonCallback() {
    //takes the user to signup page
    displaySignupPage();
}

void SocialNetworkWindow::searchCallback(){
    string s = ui->searchEdit->toPlainText().toStdString();
    ui->searchEdit->setText("");
    int id = stoi(s);
    showUser(id);
}

void SocialNetworkWindow::uploadCallback() {
    QString fileName = QFileDialog::getOpenFileName(nullptr, "Open Image File", "", "Images (*.png)");
    if (!fileName.isEmpty()) {
        QImage image;
        if (image.load(fileName)) {
            image.save("/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/pfp/temp.png");

            QPixmap p = QPixmap::fromImage(image);

            ui->signupPfp->setPixmap(p);
            ui->signupPfp->setScaledContents(true);
        }
    }
}

void SocialNetworkWindow::signupBackButtonCallback() {
    hideSignupPage();
    displayLoginPage();
}

void SocialNetworkWindow::loginButtonCallback(){

    string name = ui->loginEdit->toPlainText().toStdString();

    int id = net.getId(name);

    if(id == -1) {
        ui->errorLabel->show();
    }else {
        setLoggedUser(id);
        displayProfilePage();
    }
    ui->loginEdit->clear();
}

void SocialNetworkWindow::addButtonCallback(){
    loggedUser->addFriend(currentUser->getId());
    net.writeUsers("/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/users.txt");
}

void SocialNetworkWindow::addSuggestedCallback() {

    QWidget *w = qobject_cast<QWidget *>(sender()->parent());
    int row = 0;
    bool flag = 1;

    if(w){
        row = ui->friendSuggestionsTable->indexAt(w->pos()).row();
        flag = ui->friendSuggestionsTable->item(row,0)->text().isNull();
    }

    QTableWidgetItem* item = ui->friendSuggestionsTable->item(row, 0);
    QString userString = item->text();
    int id = std::stoi(userString.toStdString());
    loggedUser->addFriend(id);

    if(!flag) {
        ui->friendSuggestionsTable->removeRow(row);
        ui->friendSuggestionsTable->setCurrentCell(0, 0);
    }
    //if logged is being displayed then update table
    if(currentUser->getId() == loggedUser->getId()) {
        setFriendTable(loggedUser->getId());
    }
    net.writeUsers("/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/users.txt");
}

void SocialNetworkWindow::backButtonCallback() {
    showUser(loggedUser->getId());
    net.writeUsers("/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/users.txt");
}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
} // https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c

void SocialNetworkWindow::signupOKCallback() {
    // get the texts from the boxes in signup fields, and parse them into std strings and create new user
    int year, zip_int;
    string name = ui->signupNameEdit->toPlainText().toStdString();
    string yob = ui->signupYearEdit->toPlainText().toStdString();
    string zip = ui->signupZipEdit->toPlainText().toStdString();
    string friends = ui->signupFriendsEdit->toPlainText().toStdString();
    //convert some to int
    if(is_number(yob) && is_number(zip)) {
        year = stoi(yob);
        zip_int = stoi(zip);
        int id = net.numUsers();
        istringstream iss(friends);
        std::set<int> s;
        int n;
        while (iss >> n) {
            s.insert(n);
        }

        User *u = new User(id, name, year, zip_int, s);
        net.addUser(u);
        cout << "Added User:" << name << endl;
        displayLoginPage();
        //rename temp.png to the new user's pfp, else, have an empty pfp
        if (std::rename("/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/pfp/temp.png",
                        ("/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/pfp/" + std::to_string(id) + ".png").c_str()) != 0) {
            perror("file does not exist");
        }
        net.writeUsers("/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/users.txt");
        // Adds the new user to Network, and writes to file.
    } else {
        ui->signupErrorLabel->show();
    }
}

void SocialNetworkWindow::friendTableCallback(int row, int column) {

    QTableWidgetItem* user = ui->friendsTable->item(row, column);

    if (column == 1) {
        user = ui->friendsTable->item(row, 0);
    }

    //convert qtablewidgetitme to int
    QString userString = user->text();
    int id = std::stoi(userString.toStdString());

    showUser(id);
}

void SocialNetworkWindow::writePostCallback() {
    // write a post onto current viewed user's profile
    // currentUser->addPost();
    string text = "\t" + ui->writePostEdit->toPlainText().toStdString();
    Post *post = new Post(net.getNumPosts(), currentUser->getId(), text, 0);
    currentUser->addPost(post);
    // refresh the page to show the new post
    ui->writePostEdit->setText("");
    showUser(currentUser->getId());

    net.writePosts("/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/posts.txt");
    net.writeUsers("/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/users.txt");
}

void SocialNetworkWindow::suggestionTableCallback(int row, int column) {

    QTableWidgetItem* user = ui->friendSuggestionsTable->item(row, column);

    if (column == 1) {
        user = ui->friendSuggestionsTable->item(row, 0);
    }

    //convert qtablewidgetitme to int
    QString userString = user->text();
    int id = std::stoi(userString.toStdString());

    showUser(id);
}

void SocialNetworkWindow::setLoggedUser(int id) {
    loggedUser = net.getUser(id);
    currentUser = net.getUser(id);
    showUser(id);
}

void SocialNetworkWindow::showUser(int id) {
    //showUser handles friend table, friend suggestion table, and posts.
    displayProfilePage();
    currentUser = net.getUser(id);
    QString path = "/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/pfp/" + QString::number(id) + ".png";
    QPixmap pfp(path);
    int w = ui->profilePic->width();
    int h = ui->profilePic->height();
    ui->profilePic->setPixmap(pfp.scaled(w,h,Qt::KeepAspectRatioByExpanding));

    if(currentUser->getId() == loggedUser->getId()) {
        ui->profileLabel->setText(QString::fromStdString("My Profile"));
        ui->addFriendButton->hide();
        ui->friendSuggestionsLabel->show();
        ui->friendSuggestionsTable->show();
        setFriendSuggestions(id);
        ui->writePostButton->setText(QString::fromStdString(("Write a Post")));
    } else {
        ui->profileLabel->setText(QString::fromStdString(currentUser->getName()));
        string s = "Write a Post to " + net.getUser(id)->getName() + "'s Profile";
        ui->writePostButton->setText(QString::fromStdString(s));
        ui->addFriendButton->show();
        ui->friendSuggestionsLabel->hide();
        ui->friendSuggestionsTable->hide();
    }

    setFriendTable(id);
    setPosts(id);
}

void SocialNetworkWindow::setFriendTable(int id) {

    User* u = net.getUser(id);
    set<int> friendSet = u->getFriends();

    int numRows = friendSet.size();
    int numColumns = 2;

    ui->friendsTable->setRowCount(numRows);
    ui->friendsTable->setColumnCount(numColumns);

    vector<string> names;
    vector<int> ids;

    for(auto friend_: friendSet) {

        User* u = net.getUser(friend_);
        string name = u->getName();
        int id = u->getId();

        names.push_back(name);
        ids.push_back(id);
    }

    for(int row = 0; row < numRows; row++) {

        QTableWidgetItem *name = new QTableWidgetItem(QString::fromStdString(names[row]));
        QTableWidgetItem *id = new QTableWidgetItem(QString::number(ids[row]));
        ui->friendsTable->setColumnWidth(0, 40);
        ui->friendsTable->setColumnWidth(1, 200);
        ui->friendsTable->setHorizontalHeaderLabels({"ID","Name"});
        ui->friendsTable->setItem(row, 1, name);
        ui->friendsTable->setItem(row, 0, id);
    }

}

void SocialNetworkWindow::setFriendSuggestions(int id) {

    int score;
    vector<int> friendSet = net.suggestFriends(id, score);

    int numRows = friendSet.size();


    ui->friendSuggestionsTable->setRowCount(numRows);
    ui->friendSuggestionsTable->setColumnCount(3);

    vector<string> names;
    vector<int> ids;

    for(auto friend_: friendSet) {

        User* u = net.getUser(friend_);
        string name = u->getName();
        int id = u->getId();

        names.push_back(name);
        ids.push_back(id);
    }
    //construct the table
    for(int row = 0; row < numRows; row++) {

        QTableWidgetItem *name = new QTableWidgetItem(QString::fromStdString(names[row]));
        QTableWidgetItem *id = new QTableWidgetItem(QString::number(ids[row]));
        QPushButton *addFriendButton = new QPushButton("Add Friend");

        //style
        ui->friendSuggestionsTable->setColumnWidth(0, 40);
        ui->friendSuggestionsTable->setColumnWidth(1, 120);
        ui->friendSuggestionsTable->setColumnWidth(2, 100);

        ui->friendSuggestionsTable->setHorizontalHeaderLabels({"ID","Name"});
        ui->friendSuggestionsTable->setItem(row, 1, name);
        ui->friendSuggestionsTable->setItem(row, 0, id);
        ui->friendSuggestionsTable->setCellWidget(row, 2, addFriendButton);

        connect(addFriendButton, &QPushButton::clicked, this, &SocialNetworkWindow::addSuggestedCallback);
    }

    ui->friendSuggestionsTable->setHorizontalHeaderLabels({"Id", "Suggested Friend","Add as Friend?"});

}

void SocialNetworkWindow::displayLoginPage() {
    hideSignupPage();
    //show login page elements
    ui->errorLabel->hide();
    ui->loginLabel->show();
    ui->loginButton->show();
    ui->signupButton->show();
    ui->loginEdit->show();
    ui->profileLabel->hide();
    ui->friendsTable->hide();
    ui->friendsLabel->hide();
    ui->postsLabel->hide();
    ui->backButton->hide();
    ui->addFriendButton->hide();
    ui->friendSuggestionsTable->hide();
    ui->friendSuggestionsLabel->hide();
    ui->post->hide();
    ui->writePostButton->hide();
    ui->writePostEdit->hide();
    ui->profilePic->hide();
    ui->searchEdit->hide();
    ui->searchLabel->hide();
    ui->searchButton->hide();
    //ui->editProfileButton->hide();

}

void SocialNetworkWindow::displayProfilePage() {
    hideSignupPage();
    //show profile page elements
    ui->errorLabel->hide();
    ui->loginLabel->hide();
    ui->loginButton->hide();
    ui->signupButton->hide();
    ui->loginEdit->hide();
    ui->profileLabel->show();
    ui->friendsTable->show();
    ui->friendsLabel->show();
    ui->postsLabel->show();
    ui->backButton->show();
    ui->addFriendButton->show();
    ui->friendSuggestionsTable->show();
    ui->friendSuggestionsLabel->show();
    ui->post->show();
    ui->writePostButton->show();
    ui->writePostEdit->show();
    ui->profilePic->show();
    ui->searchEdit->show();
    ui->searchLabel->show();
    ui->searchButton->show();
    //ui->editProfileButton->show();
}

void SocialNetworkWindow::displaySignupPage(){
    //hide all other elements
    ui->errorLabel->hide();
    ui->loginLabel->hide();
    ui->loginButton->hide();
    ui->signupButton->hide();
    ui->loginEdit->hide();
    ui->profileLabel->hide();
    ui->friendsTable->hide();
    ui->friendsLabel->hide();
    ui->postsLabel->hide();
    ui->backButton->hide();
    ui->addFriendButton->hide();
    ui->friendSuggestionsTable->hide();
    ui->friendSuggestionsLabel->hide();
    ui->post->hide();
    ui->signupErrorLabel->hide();
    ui->writePostButton->hide();
    ui->profilePic->hide();
    ui->searchEdit->hide();
    ui->searchLabel->hide();
    ui->searchButton->hide();
    //ui->editProfileButton->hide();
    //show signuppage elements
    ui->signupFriendsEdit->show();
    ui->signupFriendsLabel->show();
    ui->signupLabel->show();
    ui->signupNameEdit->show();
    ui->signupNameLabel->show();
    ui->signupYearEdit->show();
    ui->signupYearLabel->show();
    ui->signupZipEdit->show();
    ui->signupZipLabel->show();
    ui->signupOKButton->show();
    ui->signupbackButton->show();
    ui->signupPfp->show();
    ui->signupPfp->setPixmap(QPixmap("/Users/msun/Desktop/Winter\ 2024/CSCI62/SocialNetworkGUI/pfp/1.jpeg")
                            .scaled(ui->signupPfp->width(), ui->signupPfp->height(), Qt::KeepAspectRatioByExpanding));
    ui->signupUploadButton->show();
}

void SocialNetworkWindow::hideSignupPage(){
    //hide signuppage elements
    ui->signupErrorLabel->hide();
    ui->signupButton->hide();
    ui->signupFriendsEdit->hide();
    ui->signupFriendsLabel->hide();
    ui->signupLabel->hide();
    ui->signupNameEdit->hide();
    ui->signupNameLabel->hide();
    ui->signupYearEdit->hide();
    ui->signupYearLabel->hide();
    ui->signupZipEdit->hide();
    ui->signupZipLabel->hide();
    ui->signupOKButton->hide();
    ui->signupbackButton->hide();
    ui->signupPfp->hide();
    ui->signupUploadButton->hide();
}

bool isWhitespace(unsigned char c) {
    if (c == '\t' || c == '\r' || c == '\f' || c == '\v') {
        return true;
    } else {
        return false;
    }
}

void SocialNetworkWindow::setPosts(int id) {

    string posts = net.getPostsString(id, 5, 0);
    posts.erase(std::remove_if(posts.begin(), posts.end(), isWhitespace), posts.end());
    ui->post->setText(QString::fromStdString(posts));
}

SocialNetworkWindow::~SocialNetworkWindow()
{
    delete ui;
}
