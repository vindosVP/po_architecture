# Компонентная архитектура
<!-- Состав и взаимосвязи компонентов системы между собой и внешними системами с указанием протоколов, ключевые технологии, используемые для реализации компонентов.
Диаграмма контейнеров C4 и текстовое описание. 
-->
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
-	Поиск пользователя по логину
     - входные параметры:  login
     - выходные параметры: id, login, имя, фамилия, email
-	Поиск пользователя по маске имени и фамилии
     - входные параметры: маска фамилии, маска имени
     - выходные параметры: id, login, имя, фамилия, email

### Сервис личных переписок
**API**:
- Получение личных сообщений
  - Входные параметры: id отправителя
  - Выходные параметры: список сообщений
- Отправка сообщения
  - Входные параметры: id пользователя, текст сообщения
  - Выходные параметры: отсутствуют

### Сервис групповых чатов
**API**:
- Создание чата
  - Входные параметры: название чата (опционально), список login-ов пользователей для добавления (опционально)
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
