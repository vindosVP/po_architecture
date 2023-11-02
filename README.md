# Лабораторные работы по курсу Архитектура программных систем

>**Валов Вадим** М80-208Б-22

# Задача

Необходимо реализовать бэкенд-составляющую приложения для обмена сообщениями. В приложении должна быть как возможность ведения личной переписки, так и поддержка групповых чатов.
Приложение должно содержать следующие данные:
* Пользователь
* Групповой чат 
* PtP Чат

Реализовать API:
*	Создание нового пользователя
*	Поиск пользователя по логину
*	Поиск пользователя по маске имя и фамилии
*	Создание группового чата
*	Добавление пользователя в чат
*	Добавление сообщения в групповой чат
*	Загрузка сообщений группового чата
*	Отправка PtP сообщения пользователю
*	Получение PtP списка сообщений для пользователя

# Контекст решения

```plantuml
@startuml

!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Container.puml

Person(user, "Пользователь")
Person(admin, "Администратор")

System(messenger, "Мессенджер", "Веб-сайт для обмена сообщениями")

Rel(user, messenger, "Регистрация, чтение/отправка сообщений в личных и групповых чатах, создание групповых чатов")

Rel(admin, messenger, "Мониторинг активности, помощь в случае ошибок")


@enduml
```
## Назначение систем
|Система| Описание|
|-------|---------|
| Мессенджер | Веб-интерфейс, позволяющий пользователям обмениваться сообщениями. Бэкенд сервиса реализован в виде микросервисной архитектуры |

# Компонентная архитектура

## Компонентная диаграмма

```plantuml
@startuml
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Container.puml

AddElementTag("microService", $shape=EightSidedShape(), $bgColor="CornflowerBlue", $fontColor="white", $legendText="microservice")
AddElementTag("storage", $shape=RoundedBoxShape(), $bgColor="lightSkyBlue", $fontColor="white")

Person(user, "Пользователь")

System_Ext(web_site, "Клиентский веб-сайт", "HTML, CSS, JavaScript, React", "Веб-интерфейс")

System_Boundary(conference_site, "Сайт") {
   'Container(web_site, "Клиентский веб-сайт", ")
   Container(client_service, "Сервис авторизации", "C++", "Сервис управления пользователями", $tags = "microService")    
   Container(post_service, "Сервис личных переписок", "C++", "Сервис управления личными сообщениями", $tags = "microService") 
   Container(blog_service, "Сервис групповых чатов", "C++", "Сервис управления групповыми чатами", $tags = "microService")   
   ContainerDb(db, "База данных", "MySQL", "Хранение данных сообщениях, чатах и пользователях", $tags = "storage")
   
}

Rel(user, web_site, "Регистрация, чтение/отправка сообщений в личных и групповых чатах, создание групповых чатов")

Rel(web_site, client_service, "Работа с пользователями", "localhost/person")
Rel(client_service, db, "INSERT/SELECT/UPDATE", "SQL")

Rel(web_site, post_service, "Работа с личными сообщениями", "localhost/chats")
Rel(post_service, db, "INSERT/SELECT/UPDATE", "SQL")

Rel(web_site, blog_service, "Работа с группами", "localhost/groups")
Rel(blog_service, db, "INSERT/SELECT/UPDATE", "SQL")

@enduml
```
## Список компонентов  

### Сервис авторизации
**API**:
-	Создание нового пользователя
      - входные параметры: login, пароль, имя, фамилия, email
      - выходные параметры: отсутствуют
-	Поиск пользователя по ид
     - входные параметры:  id
     - выходные параметры: login, пароль, имя, фамилия, email
-	Поиск пользователя по маске имени и фамилии
     - входные параметры: имя, фамилия
     - выходные параметры: login, пароль, имя, фамилия, email

### Сервис чатов
**API**:
- Получение личных сообщений
  - Входные параметры: id отправителя
  - Выходные параметры: список сообщений
- Отправка сообщения
  - Входные параметры: id пользователя, текст сообщения
  - Выходные параметры: отсутствуют
- Создание чата
  - Входные параметры: название чата
  - Выходные параметры: id чата
- Добавление пользователя в чат
  - Входные параметры: id пользователя, id чата
  - Выходные параметры: отсутствуют
- Добавление сообщения в чат
  - Входные параметры: id чата, текст сообщения
  - Выходные парамтеры: отсутствуют
- Получения сообщений из группового чата
  - Входные параметры: id чата
  - Выходные параметры: список сообщений

# Документация

* Документацию в формате OpenAPI можно найти в `docs/swagger`
* Коллекцию запросов и окружение можно найти в `docs/postman`
* Документация Postman доступна по [ссылке](https://documenter.getpostman.com/view/20758355/2s9YXe6P7m) 
